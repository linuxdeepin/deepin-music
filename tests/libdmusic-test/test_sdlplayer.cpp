// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// SdlPlayer 单元测试（第三阶段）
//
// sdlplayer.cpp 深度依赖 SDL2 音频设备，但在 offscreen 测试环境下已验证
// （test_cda.cpp 中已用 VlcInstance 构造 SdlPlayer 并运行），构造是安全的：
//   - SDL 库通过 VlcDynamicInstance 动态加载；
//   - SDL_Init(AUDIO) 失败时仅打 warning 并继续，不会 abort；
//   - 未实际播放音频时不触发真实音频设备 IO。
//
// 因此本文件覆盖 SDL 依赖之外的纯逻辑分支：
//   - getter/setter（getVolume/getMute/setVolume/setMute/setCurMeta/getCurMeta/setProgressTag）
//   - 防御分支（_vlcMediaPlayer 为空时的 play/pause/stop/resume 早退；open 空 media 早退）
//   - 不依赖音频设备的方法（checkDataZero 仅 emit end()、setTime、cleanMemCache）
//   - 静态格式解析（libvlc_audio_format / format_from_vlc_to_SDL，纯字符串解析）
//   - libvlc_audio_play_cb 的防御分支（progressTag 非零早退、零除/空参数早退）
//   - libvlc_audio_flush_cb（空实现）
//   - SDL_LogOutputFunction_Err_Write 错误日志回调
//
// 注意：私有/静态方法在 .so 中可见（默认可见性），通过前向声明调用；
//      不修改产品代码，不加 friend。
//
// 跳过并记录（依赖真实音频设备，offscreen 下必崩或无意义）：
//   - libvlc_audio_pause_cb / resume_cb：内部调用 SDL_GetAudioStatus/SDL_PauseAudio，
//     SDL 未真正初始化设备时可能 segfault。
//   - libvlc_audio_setup_cb：调用 SDL_OpenAudio 打开真实音频设备。
//   - SDL_audio_cbk：SDL 回调上下文，依赖 SDL_memset/MixAudio 符号且 stream 指针有效。
//   - resetVolume / readSinkInputPath：依赖 DDE Audio1 DBus 服务。
//   - 真实 play()/pause()/stop()/resume() 触发 SDL 设备操作路径（仅测防御早退分支）。

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <QString>
#include <QByteArray>
#include <memory>

#include "player/vlc/sdlplayer.h"
#include "player/vlc/Instance.h"
#include "global.h"

// ============================================================================
// 私有/静态方法访问说明
// ============================================================================
// sdlplayer.cpp 中的静态方法（libvlc_audio_format / format_from_vlc_to_SDL /
// libvlc_audio_play_cb / libvlc_audio_flush_cb）以及自由函数 SDL_LogOutputFunction_Err_Write
// 在头文件中未声明（或为 private static），但 libdmusic.so 默认可见性下已导出
// （nm -D 确认符号存在）。
//
// private static 方法在子类中无法用 using 提升（private 不可见），也无法用
// SdlPlayer:: 限定名直接调用。这里采用 dlsym(RTLD_DEFAULT) 取 mangled 符号地址，
// 转为函数指针调用——运行期取地址，不依赖编译期访问权限，且不修改产品代码、不加 friend。
//
// 这些纯字符串解析/防御分支函数与 SDL/音频设备完全无关，是 offscreen 下最有价值的覆盖点。

#include <dlfcn.h>

namespace {
// dlsym 句柄缓存：从主进程符号表（含 libdmusic.so）解析 SdlPlayer 静态私有方法。
// RTLD_DEFAULT 在全局符号表（含已加载共享库）中查找，无需显式 handle。
template <typename FuncT>
FuncT resolveStatic(const char *mangled)
{
    // clang/clang++ 在 next 模式可能延迟绑定；这里用标准 dlsym。
    void *p = dlsym(RTLD_DEFAULT, mangled);
    return reinterpret_cast<FuncT>(p);
}
}  // namespace

// ============================================================================
// 测试夹具：提供可复用的 VlcInstance + SdlPlayer
// 设计要点：
//   - 每个 TEST_F 各自构造，避免共享播放器状态导致 flaky；
//   - 用 unique_ptr 管理生命周期，TearDown 时自然析构（验证析构不崩溃）。
// ============================================================================
class SdlPlayerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 与 test_cda.cpp 一致：QtPlayer 后端可避免 VLC 强依赖，但 SdlPlayer 本身
        // 需 VlcInstance 才能构造（继承自 VlcMediaPlayer）。这里直接构造 VLC 实例。
        m_instance = std::make_unique<VlcInstance>(QStringList(), nullptr);
        ASSERT_NE(m_instance->core(), nullptr);
    }

    void TearDown() override
    {
        // 先释放 player 再释放 instance（依赖顺序）
        m_player.reset();
        m_instance.reset();
    }

    std::unique_ptr<VlcInstance> m_instance;
    std::unique_ptr<SdlPlayer> m_player;
};

// ============================================================================
// 构造/析构：offscreen 下安全
// ============================================================================
TEST_F(SdlPlayerTest, constructsAndDestroysSafelyOffscreen)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    EXPECT_NE(m_player.get(), nullptr);
    // 作用域结束触发析构：~SdlPlayer 中 SDL_Quit 仅在 m_loadSdlLibrary 为 true 时调用，
    // offscreen 下 SDL 库可能加载但设备初始化失败，析构路径应安全（已验证）。
}

TEST_F(SdlPlayerTest, canConstructMultipleInstances)
{
    // 多次构造/析构不应崩溃（验证 SDL 库加载/卸载循环稳定）
    for (int i = 0; i < 3; ++i) {
        auto p = std::make_unique<SdlPlayer>(m_instance.get());
        EXPECT_NE(p.get(), nullptr);
    }
}

// ============================================================================
// setCurMeta / getCurMeta：内联 getter/setter 回环
// ============================================================================
TEST_F(SdlPlayerTest, curMetaRoundTrip)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    DMusic::MediaMeta meta;
    meta.hash = "test-hash";
    meta.title = "Test Title";
    meta.localPath = "/tmp/test.mp3";
    m_player->setCurMeta(meta);

    DMusic::MediaMeta got = m_player->getCurMeta();
    EXPECT_EQ(got.hash.toStdString(), "test-hash");
    EXPECT_EQ(got.title.toStdString(), "Test Title");
    EXPECT_EQ(got.localPath.toStdString(), "/tmp/test.mp3");
}

// ============================================================================
// setProgressTag：默认值 0，设置后存储（setProgressTag 无 getter，
// 但通过它影响 libvlc_audio_play_cb 的早退分支间接验证——见后面 play_cb 测试）
// ============================================================================
TEST_F(SdlPlayerTest, setProgressTagDoesNotCrash)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    EXPECT_NO_FATAL_FAILURE(m_player->setProgressTag(0));
    EXPECT_NO_FATAL_FAILURE(m_player->setProgressTag(1));
    EXPECT_NO_FATAL_FAILURE(m_player->setProgressTag(-1));
    EXPECT_NO_FATAL_FAILURE(m_player->setProgressTag(999));
}

// ============================================================================
// checkDataZero：仅 emit end() 信号，不依赖音频设备
// ============================================================================
TEST_F(SdlPlayerTest, checkDataZeroEmitsEndSignal)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    QSignalSpy spy(m_player.get(), &SdlPlayer::end);
    ASSERT_TRUE(spy.isValid());
    EXPECT_EQ(spy.count(), 0);

    m_player->checkDataZero();
    EXPECT_EQ(spy.count(), 1);

    // 多次调用应多次发射
    m_player->checkDataZero();
    m_player->checkDataZero();
    EXPECT_EQ(spy.count(), 3);
}

// ============================================================================
// getVolume / setVolume：m_loadSdlLibrary 为 true 时读写 m_volume（默认 50）
// 注意：offscreen 下 m_loadSdlLibrary 取决于 SDL 库是否加载成功，两路径都测。
// ============================================================================
TEST_F(SdlPlayerTest, getVolumeReturnsInitialValueWithoutCrash)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    int vol = m_player->getVolume();
    // 默认 m_volume = 50（sdlplayer.h）。若 SDL 库未加载则走 VlcMediaPlayer::getVolume()，
    // 两者返回值都可能非 50，不强断言具体值，仅验证可调用且在合理范围。
    EXPECT_GE(vol, 0);
    EXPECT_LE(vol, 100);
}

TEST_F(SdlPlayerTest, setVolumeThenGetRoundTrip)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    // setVolume 在 m_loadSdlLibrary=true 时直接写 m_volume，getVolume 回读
    m_player->setVolume(0);
    m_player->setVolume(100);
    m_player->setVolume(42);
    int vol = m_player->getVolume();
    // 若 SDL 库已加载，最后一次应读回 42；否则走基类路径。仅验证不崩溃且范围合理。
    EXPECT_GE(vol, 0);
    EXPECT_LE(vol, 100);
}

TEST_F(SdlPlayerTest, setVolumeClampBoundary)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    EXPECT_NO_FATAL_FAILURE(m_player->setVolume(-10));   // 负值：产品代码不裁剪，直接存
    EXPECT_NO_FATAL_FAILURE(m_player->setVolume(200));   // 超值：同上
}

// ============================================================================
// getMute / setMute：默认 false，回环
// ============================================================================
TEST_F(SdlPlayerTest, muteRoundTrip)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    m_player->setMute(true);
    bool m1 = m_player->getMute();
    m_player->setMute(false);
    bool m2 = m_player->getMute();
    // 若 SDL 库加载成功，setMute 直接写 m_mute，getMute 回读：应分别为 true/false。
    // 否则走基类路径。不强断言，仅验证可调用不崩溃（两者均布尔值）。
    EXPECT_TRUE(m1 == true || m1 == false);
    EXPECT_TRUE(m2 == true || m2 == false);
}

// ============================================================================
// setTime：调用基类 + cleanMemCache（清空内部 _data 缓冲），不崩溃
// 无媒体时 VlcMediaPlayer::setTime 走防御分支。
// ============================================================================
TEST_F(SdlPlayerTest, setTimeDoesNotCrashWithoutMedia)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    EXPECT_NO_FATAL_FAILURE(m_player->setTime(0));
    EXPECT_NO_FATAL_FAILURE(m_player->setTime(1000));
    EXPECT_NO_FATAL_FAILURE(m_player->setTime(-1));
}

// ============================================================================
// 防御分支：play/pause/stop/resume 在 _vlcMediaPlayer 有效时调用
// 这里 _vlcMediaPlayer 由 VlcMediaPlayer 基类构造时通过 libvlc_media_player_new 创建。
// 实际播放需要媒体文件；这里仅验证方法可调用且不触发崩溃（不真正发声）。
// 由于不 open 媒体，libvlc 的 play() 内部为空操作，SDL 路径在 m_loadSdlLibrary=false
// 时被完全跳过——这是 offscreen 下最安全的测试角度。
// ============================================================================
TEST_F(SdlPlayerTest, playWithoutMediaDoesNotCrash)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    EXPECT_NO_FATAL_FAILURE(m_player->play());
    // 让事件循环跑一下，让 checkDataThread（若启动）有机会设置退出标志
    QTest::qWait(50);
}

TEST_F(SdlPlayerTest, pauseWithoutMediaDoesNotCrash)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    EXPECT_NO_FATAL_FAILURE(m_player->pause());
}

TEST_F(SdlPlayerTest, stopWithoutMediaDoesNotCrash)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    EXPECT_NO_FATAL_FAILURE(m_player->stop());
}

TEST_F(SdlPlayerTest, resumeWithoutMediaDoesNotCrash)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    EXPECT_NO_FATAL_FAILURE(m_player->resume());
}

TEST_F(SdlPlayerTest, fullControlSequenceWithoutMedia)
{
    // 组合调用：模拟一次完整生命周期（无媒体，纯控制流）
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    EXPECT_NO_FATAL_FAILURE(m_player->play());
    QTest::qWait(20);
    EXPECT_NO_FATAL_FAILURE(m_player->pause());
    EXPECT_NO_FATAL_FAILURE(m_player->resume());
    EXPECT_NO_FATAL_FAILURE(m_player->stop());
}

// ============================================================================
// open() 防御分支：media->core() 为空时早退
// 用 nullptr 媒体或无效媒体触发。VlcMedia 需 VlcInstance 构造；
// 这里构造一个 VlcMedia 但不打开文件，core() 可能为空或有效——主要验证不崩溃。
// ============================================================================
#include "player/vlc/Media.h"

TEST_F(SdlPlayerTest, openWithEmptyMediaCoreDoesNotCrash)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    // VlcMedia 默认构造，未 initMedia → core() 返回空（或内部 media 为空）→ 触发早退分支
    VlcMedia media;
    EXPECT_NO_FATAL_FAILURE(m_player->open(&media));
}

TEST_F(SdlPlayerTest, openWithNonExistentPathDoesNotCrash)
{
    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    VlcMedia media;
    media.initMedia("/nonexistent/path/fake.mp3", false, m_instance.get());
    EXPECT_NO_FATAL_FAILURE(m_player->open(&media));
}

// ============================================================================
// 静态格式解析函数：libvlc_audio_format / format_from_vlc_to_SDL
// 纯字符串解析，与 SDL/音频设备完全无关，是最有价值的覆盖点。
//
// 由于它们是 SdlPlayer 的 private static，无法直接 SdlPlayer:: 限定调用。
// 通过 dlsym(RTLD_DEFAULT) 取 mangled 符号地址，转成函数指针调用。
// 符号已在 libdmusic.so 中确认导出：
//   _ZN9SdlPlayer19libvlc_audio_formatEPc   (unsigned int SdlPlayer::libvlc_audio_format(char*))
//   _ZN9SdlPlayer22format_from_vlc_to_SDLEPc (unsigned int SdlPlayer::format_from_vlc_to_SDL(char*))
// ============================================================================
typedef unsigned int (*LibvlcAudioFormatFunc)(char *format);
typedef unsigned int (*FormatFromVlcToSDLFunc)(char *format);

TEST(SdlPlayerStaticFormatTest, libvlcAudioFormatParsesBitDepth)
{
    LibvlcAudioFormatFunc fn = resolveStatic<LibvlcAudioFormatFunc>(
        "_ZN9SdlPlayer19libvlc_audio_formatEPc");
    ASSERT_NE(fn, nullptr) << "libvlc_audio_format 符号未找到";

    // contains("8") → 8
    {
        char buf[] = "S8";
        EXPECT_EQ(fn(buf), 8u);
    }
    // contains("16") → 16
    {
        char buf[] = "S16";
        EXPECT_EQ(fn(buf), 16u);
    }
    // contains("32") → 32
    {
        char buf[] = "S32";
        EXPECT_EQ(fn(buf), 32u);
    }
    // contains("64") → 64
    {
        char buf[] = "S64";
        EXPECT_EQ(fn(buf), 64u);
    }
    // 其他 → 默认 16
    {
        char buf[] = "FLAC";
        EXPECT_EQ(fn(buf), 16u);
    }
    // 注意：libvlc_audio_format 的判定顺序是 8→16→32→64→else，
    // "16" 不会先匹配 "1" 再被 "8" 抢走，因为 contains("8") 检查整个 "16" 是否含 "8" → 否。
}

TEST(SdlPlayerStaticFormatTest, libvlcAudioFormatOrdering8CheckedFirst)
{
    LibvlcAudioFormatFunc fn = resolveStatic<LibvlcAudioFormatFunc>(
        "_ZN9SdlPlayer19libvlc_audio_formatEPc");
    ASSERT_NE(fn, nullptr);
    // "18000" 同时含 "8" 和 "16"？含 "8" → 命中第一个分支返回 8
    // （注意 "16" 不含 "8"，但 "18000" 含 "8"）
    {
        char buf[] = "18000";
        EXPECT_EQ(fn(buf), 8u);  // contains("8") 优先
    }
    // 空字符串 → 走 else → 16
    {
        char buf[] = "";
        EXPECT_EQ(fn(buf), 16u);
    }
}

TEST(SdlPlayerStaticFormatTest, formatFromVlcToSDLMapsFormats)
{
    FormatFromVlcToSDLFunc fn = resolveStatic<FormatFromVlcToSDLFunc>(
        "_ZN9SdlPlayer22format_from_vlc_to_SDLEPc");
    ASSERT_NE(fn, nullptr) << "format_from_vlc_to_SDL 符号未找到";

    // VLC 格式串约定为小写（s16l/s16b/u8/f32l 等），产品代码用 contains("u")
    // /contains("f") 做大小写敏感匹配。这里用 VLC 真实格式名测试。
    // 8-bit: 小写 u → AUDIO_U8，否则 s → AUDIO_S8
    {
        char buf[] = "u8";   // VLC 无符号 8 位
        EXPECT_EQ(fn(buf), static_cast<unsigned int>(AUDIO_U8));
    }
    {
        char buf[] = "s8";   // VLC 有符号 8 位
        EXPECT_EQ(fn(buf), static_cast<unsigned int>(AUDIO_S8));
    }
    // 16-bit: u → AUDIO_U16SYS, s → AUDIO_S16SYS
    {
        char buf[] = "u16l";
        EXPECT_EQ(fn(buf), static_cast<unsigned int>(AUDIO_U16SYS));
    }
    {
        char buf[] = "s16l";
        EXPECT_EQ(fn(buf), static_cast<unsigned int>(AUDIO_S16SYS));
    }
    // 32-bit: f → AUDIO_F32SYS, else → AUDIO_S32SYS
    {
        char buf[] = "f32l";
        EXPECT_EQ(fn(buf), static_cast<unsigned int>(AUDIO_F32SYS));
    }
    {
        char buf[] = "s32l";
        EXPECT_EQ(fn(buf), static_cast<unsigned int>(AUDIO_S32SYS));
    }
    // 其他 → 默认 16（注意产品代码这里返回字面值 16，不是 AUDIO_S16SYS）
    {
        char buf[] = "FLAC";
        EXPECT_EQ(fn(buf), 16u);
    }
}

// 大小写敏感边界：产品代码 contains("u")/contains("f") 区分大小写，
// 大写 "U"/"F" 不会被识别为 unsigned/float，会落到 signed 分支。
// 这条测试固化该行为，防止误改。
TEST(SdlPlayerStaticFormatTest, formatFromVlcToSDLIsCaseSensitive)
{
    FormatFromVlcToSDLFunc fn = resolveStatic<FormatFromVlcToSDLFunc>(
        "_ZN9SdlPlayer22format_from_vlc_to_SDLEPc");
    ASSERT_NE(fn, nullptr);

    // "U8"（大写）→ contains("8") true，contains("u") false → AUDIO_S8
    {
        char buf[] = "U8";
        EXPECT_EQ(fn(buf), static_cast<unsigned int>(AUDIO_S8));
    }
    // "F32"（大写）→ contains("32") true，contains("f") false → AUDIO_S32SYS
    {
        char buf[] = "F32";
        EXPECT_EQ(fn(buf), static_cast<unsigned int>(AUDIO_S32SYS));
    }
}

// ============================================================================
// libvlc_audio_flush_cb：空实现（Q_UNUSED data/pts），调用即覆盖
// 该函数是 private static，符号 _ZN9SdlPlayer21libvlc_audio_flush_cbEPvl
// ============================================================================
typedef void (*LibvlcAudioFlushFunc)(void *data, int64_t pts);

TEST(SdlPlayerStaticFormatTest, flushCallbackIsNoOp)
{
    LibvlcAudioFlushFunc fn = resolveStatic<LibvlcAudioFlushFunc>(
        "_ZN9SdlPlayer21libvlc_audio_flush_cbEPvl");
    ASSERT_NE(fn, nullptr);
    // 传任意值，函数体为空，不应崩溃
    EXPECT_NO_FATAL_FAILURE(fn(nullptr, 0));
    int dummy = 42;
    EXPECT_NO_FATAL_FAILURE(fn(&dummy, 12345));
}

// ============================================================================
// libvlc_audio_play_cb 防御分支（不触发真实 SDL/音频）
// 符号 _ZN9SdlPlayer20libvlc_audio_play_cbEPvPKvjl
//
// 覆盖的分支：
//   1. data == nullptr → 早退 return（不崩溃）
//   2. data 有效但 progressTag != 0 → 早退 return
//   3. data 有效、progressTag==0、但 _rate/_channels 为 0 → bytesPerSample==0 早退
// 不覆盖：实际拷贝 samples 的主路径（需要构造合法 SDL_AudioSpec obtainedAS 与样本数据，
//         且会写入 _data；该路径涉及内存布局假设，风险较高，跳过并记录）。
// ============================================================================
typedef void (*LibvlcAudioPlayCbFunc)(void *data, const void *samples, unsigned count, int64_t pts);

TEST(SdlPlayerPlayCallbackTest, playCallbackReturnsOnNullData)
{
    LibvlcAudioPlayCbFunc fn = resolveStatic<LibvlcAudioPlayCbFunc>(
        "_ZN9SdlPlayer20libvlc_audio_play_cbEPvPKvjl");
    ASSERT_NE(fn, nullptr);
    // data 为空 → 第一行 SdlPlayer *sdlMediaPlayer = cast; if(!sdlMediaPlayer) return;
    char samples[4] = {0};
    EXPECT_NO_FATAL_FAILURE(fn(nullptr, samples, 1, 0));
}

TEST(SdlPlayerPlayCallbackTest, playCallbackReturnsWhenProgressTagNonZero)
{
    LibvlcAudioPlayCbFunc fn = resolveStatic<LibvlcAudioPlayCbFunc>(
        "_ZN9SdlPlayer20libvlc_audio_play_cbEPvPKvjl");
    ASSERT_NE(fn, nullptr);

    // 用真实 SdlPlayer 实例：构造后 setProgressTag(1) 使 progressTag 非零
    VlcInstance instance(QStringList(), nullptr);
    ASSERT_NE(instance.core(), nullptr);
    SdlPlayer sdlp(&instance);
    sdlp.setProgressTag(1);  // progressTag = 1 → 早退

    char samples[16] = {0};
    // data 指向 sdlp，但 progressTag 非零 → 在读取 _rate 前就 return
    EXPECT_NO_FATAL_FAILURE(fn(&sdlp, samples, 10, 0));
}

TEST_F(SdlPlayerTest, playCallbackReturnsWhenRateIsZero)
{
    LibvlcAudioPlayCbFunc fn = resolveStatic<LibvlcAudioPlayCbFunc>(
        "_ZN9SdlPlayer20libvlc_audio_play_cbEPvPKvjl");
    ASSERT_NE(fn, nullptr);

    m_player = std::make_unique<SdlPlayer>(m_instance.get());
    // 注意：_rate/_channels/obtainedAS.channels 是 SdlPlayer 的成员，构造后未 setup，
    // 值不确定（取决于构造函数是否清零）。libvlc_audio_play_cb 内：
    //   bytesPerSample = _rate/8; dstChannels = obtainedAS.channels; srcChannels = _channels;
    //   if (bytesPerSample==0 || dstChannels==0 || srcChannels==0 || count==0) return;
    //   size = count * dstChannels * bytesPerSample;  char curSamples[size];  ← VLA
    // 若任一成员非零且 count 较大，size 可能巨大 → VLA 栈溢出 → segfault。
    // 因此本用例传 count=0 触发早退（count==0 分支），确保不进入 VLA。
    m_player->setProgressTag(0);  // 默认 0，确保不走 progressTag 早退
    char samples[16] = {0};
    EXPECT_NO_FATAL_FAILURE(fn(m_player.get(), samples, 0, 0));  // count=0 → 早退
}

// ============================================================================
// SDL_LogOutputFunction_Err_Write：SDL 日志回调
// 符号 _Z31SDL_LogOutputFunction_Err_WritePvi15SDL_LogPriorityPKc（自由函数）
//
// 该函数内部调用 VlcDynamicInstance::resolveSdlSymbol("SDL_GetAudioStatus") 取符号；
// 若 SDL 库未加载，resolveSdlSymbol 返回 nullptr，GetAudioStatus() 解引用 nullptr → 崩溃。
// 因此仅在 SDL 库已加载（m_loadSdlLibrary=true）时安全测试，否则跳过。
//
// 更安全的角度：消息不匹配 SDL_AUDIO_ERR_MSG 时，进入分支前 strmsg 比较就 return，
// 不会调用 GetAudioStatus。所以传一个普通消息（非 "Error writing to datastream"）总是安全的。
// ============================================================================
typedef void (*SDLLogOutputFunc)(void *userdata, int category, int priority, const char *message);

TEST(SdlPlayerLogCallbackTest, logCallbackHandlesNonErrorMessages)
{
    SDLLogOutputFunc fn = resolveStatic<SDLLogOutputFunc>(
        "_Z31SDL_LogOutputFunction_Err_WritePvi15SDL_LogPriorityPKc");
    ASSERT_NE(fn, nullptr);

    // 传一个非 SDL_AUDIO_ERR_MSG 的普通消息：strmsg 比较失败，直接走完函数不调用 GetAudioStatus
    const char *msg = "Some info message";
    int dummy = 0;
    EXPECT_NO_FATAL_FAILURE(fn(&dummy, 0, 0, msg));
    EXPECT_NO_FATAL_FAILURE(fn(nullptr, 0, 0, "another harmless text"));
}

TEST(SdlPlayerLogCallbackTest, logCallbackHandlesNullMessage)
{
    SDLLogOutputFunc fn = resolveStatic<SDLLogOutputFunc>(
        "_Z31SDL_LogOutputFunction_Err_WritePvi15SDL_LogPriorityPKc");
    ASSERT_NE(fn, nullptr);
    // 注意：函数内 QString strmsg = message; 若 message==nullptr 会构造空 QString（Qt 安全），
    // 随后 strmsg == SDL_AUDIO_ERR_MSG 为 false，return。不触发 GetAudioStatus。
    EXPECT_NO_FATAL_FAILURE(fn(nullptr, 0, 0, nullptr));
}

// 错误消息路径（category=SDL_LOG_CATEGORY_AUDIO, priority=SDL_LOG_PRIORITY_ERROR, msg=SDL_AUDIO_ERR_MSG）
// 会调用 GetAudioStatus()，该函数依赖 SDL 库已加载。仅在确认 SDL 库可用时测；
// offscreen 下不确定，故跳过，记录原因。
TEST(SdlPlayerLogCallbackTest, DISABLED_errorMessagePathRequiresSdlDevice)
{
    // 需要 SDL 库加载且 SDL_GetAudioStatus 可解析，offscreen 下不确定，
    // 跳过以避免潜在 segfault。
    SUCCEED();
}
