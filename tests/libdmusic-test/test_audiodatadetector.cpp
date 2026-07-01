// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// AudioDataDetector 的单元测试：构造/析构、槽函数、缓存查询、重采样逻辑、
// 真实音频解码链路（run()/resample()）。
// AudioDataDetector 继承自 QThread，依赖 ffmpeg（通过 DynamicLibraries 动态加载）。
// 测试策略：
//   - 用 testdata/sample.mp3 触发完整解码链路：onBufferDetector → start() → run()
//     → ffmpeg 解码 → resample() 降采样 → emit audioBufferFromThread（queued）→ audioBuffer。
//     这是覆盖 run()/resample() 核心算法（盲区主体）的关键。
//   - 缓存命中：先解码写缓存(.dat)，再用相同 hash 触发 queryCacheExisted 命中分支。
//   - engineType=0(Qt) 分支：onBufferDetector 走 queryCacheExisted 的 default_music.dat 兜底。
//   - stopFlag 截断：onBufferDetector 后立即 onClearBufferDetector，覆盖 resample 的 forceQuit 分支。
//   - 防御分支：空路径、不存在的路径、相同 hash 忽略。
// 线程安全：所有触发 run() 的用例结束时 wait()/确保线程退出，避免析构竞态。
// 缓存目录说明：resample() 写文件用 QStandardPaths::CacheLocation，
//              queryCacheExisted() 读 DmGlobal::cachePath() —— 两处路径需分别处理。

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QSignalSpy>
#include <QTest>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QThread>
#include <QStandardPaths>
#include <memory>

#include "audiodatadetector.h"
#include "dynamiclibraries.h"
#include "global.h"

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "."
#endif

// ============================================================================
// 辅助函数
// ============================================================================
namespace {

// testdata/sample.mp3 的绝对路径（编译期注入宏）
QString sampleMp3Path()
{
    return QString(TEST_DATA_DIR) + "/sample.mp3";
}

// resample() 实际写入的系统缓存目录（QStandardPaths::CacheLocation/wave）
// 注意：resample 用 QStandardPaths::CacheLocation，而 queryCacheExisted 用 DmGlobal::cachePath()
QString systemWaveDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/wave";
}

// DmGlobal::cachePath() 对应的缓存目录（queryCacheExisted 读取的位置）
QString dmCacheWaveDir()
{
    return DmGlobal::cachePath() + "/wave";
}

// 删除指定 hash 的系统缓存文件(.dat)，保证测试从干净状态开始
void removeSystemCacheFor(const QString &hash)
{
    QDir().mkpath(systemWaveDir());
    QFile::remove(systemWaveDir() + QString("/%1.dat").arg(hash));
}

// 删除指定 hash 的 DmGlobal 缓存文件(.dat)
void removeDmCacheFor(const QString &hash)
{
    QDir d;
    d.mkpath(dmCacheWaveDir());
    QFile::remove(dmCacheWaveDir() + QString("/%1.dat").arg(hash));
}

// RAII：测试期间把播放引擎类型切到 type，析构时还原为 0，避免污染其它用例。
// onBufferDetector 中 playbackEngineType()==1（VLC/FFmpeg）才启动解码线程。
class EngineTypeGuard
{
public:
    explicit EngineTypeGuard(int type) { DmGlobal::setPlaybackEngineType(type); }
    ~EngineTypeGuard() { DmGlobal::setPlaybackEngineType(0); }
};

// 确保线程安全退出的辅助：wait + clear
void ensureThreadStopped(AudioDataDetector *det)
{
    if (det && det->isRunning()) {
        det->wait(5000);
    }
}
}

// ============================================================================
// 构造/析构：不崩溃
// ============================================================================
TEST(AudioDataDetectorConstructionTest, constructsAndDestroysWithoutCrash)
{
    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    EXPECT_NE(detector.get(), nullptr);
}

TEST(AudioDataDetectorConstructionTest, destructorWaitsForThread)
{
    AudioDataDetector *detector = new AudioDataDetector();
    // 析构时 ~AudioDataDetector 会等待线程结束（即使线程未启动也不崩溃）
    delete detector;
    SUCCEED();
}

// ============================================================================
// onClearBufferDetector：不崩溃，清理内部状态
// ============================================================================
TEST(AudioDataDetectorClearTest, onClearBufferDetectorDoesNotCrash)
{
    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    // 线程未启动时调用
    detector->onClearBufferDetector();
    // 清理后 m_curPath/m_curHash 应为空（间接验证：再次调用不会因残留状态崩溃）
    detector->onClearBufferDetector();
    SUCCEED();
}

// ============================================================================
// onBufferDetector：防御性分支
// ============================================================================
TEST(AudioDataDetectorBufferTest, onBufferDetectorWithEmptyPathDoesNotCrash)
{
    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    // 空路径触发 path.isEmpty() 分支，线程不会启动
    detector->onBufferDetector("", "hash_empty_path");
    EXPECT_FALSE(detector->isRunning());
    SUCCEED();
}

TEST(AudioDataDetectorBufferTest, onBufferDetectorWithNonexistentPathDoesNotCrash)
{
    EngineTypeGuard guard(1);  // FFmpeg 模式
    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    // 不存在的文件路径，ffmpeg 打开失败后清理退出，不崩溃
    detector->onBufferDetector("/nonexistent/path/audio.mp3", "hash_nonexistent");
    // 等待线程结束（线程会启动但 run() 中 format_open_input 失败后 return）
    ensureThreadStopped(detector.get());
    EXPECT_FALSE(detector->isRunning());
}

TEST(AudioDataDetectorBufferTest, onBufferDetectorWithSameHashIgnored)
{
    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    // 第一次设置 hash，第二次相同 hash 命中 (hash == m_curHash) 分支直接返回
    // 第一次用空路径不会启动线程，m_curHash 仍被设置
    detector->onBufferDetector("", "same_hash_first");
    // 再次用相同 hash：命中 hash 匹配分支
    detector->onBufferDetector("/any/path.mp3", "same_hash_first");
    SUCCEED();
}

// ============================================================================
// run() + resample() 真实解码链路：sample.mp3 触发完整波形计算
// 这是覆盖率提升的核心：覆盖 run() 的 ffmpeg 解码循环、resample() 的
// 降采样分支（buffer>1000）+ 归一化 + 缓存写入，以及 audioBufferFromThread 信号。
// ============================================================================
TEST(AudioDataDetectorDecodeTest, sampleMp3TriggersFullResampleAndEmitsSignal)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path())) << "testdata/sample.mp3 missing";

    EngineTypeGuard guard(1);  // 必须为 1 才启动解码线程
    // 清理可能的旧缓存，保证走解码路径而非缓存命中
    const QString hash = "decode_full_hash_001";
    removeSystemCacheFor(hash);

    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());

    // 监听 worker 线程直接发出的信号（resample 末尾 emit）
    QSignalSpy spyFromThread(detector.get(), &AudioDataDetector::audioBufferFromThread);
    // 监听经 QueuedConnection 转发到主线程的信号
    QSignalSpy spyBuffer(detector.get(), &AudioDataDetector::audioBuffer);

    detector->onBufferDetector(sampleMp3Path(), hash);

    // wait() 内部运行事件循环，既等待信号又驱动 queued connection 投递
    ASSERT_TRUE(spyFromThread.wait(20000)) << "audioBufferFromThread 未在超时内触发";
    ensureThreadStopped(detector.get());

    // 验证 resample 产出了非空波形数据（buffer>1000 走降采样分支）
    ASSERT_EQ(spyFromThread.count(), 1);
    const QVector<float> buf = spyFromThread.takeFirst().at(0).value<QVector<float>>();
    EXPECT_GT(buf.size(), 0) << "resample 应输出归一化波形数据";

    // 给 queued connection 一点时间，验证 audioBuffer 也被转发
    QCoreApplication::processEvents();
    if (spyBuffer.count() == 0) {
        QTest::qWait(50);  // 容错：偶发需要再处理一轮事件
        QCoreApplication::processEvents();
    }
    EXPECT_GE(spyBuffer.count(), 1) << "audioBuffer 信号经 queued connection 应被转发";

    // 验证 resample 写入了缓存文件（非 forceQuit 分支）
    const QString cacheFile = systemWaveDir() + QString("/%1.dat").arg(hash);
    EXPECT_TRUE(QFile::exists(cacheFile)) << "resample 应写入 .dat 缓存";
}

// ============================================================================
// 缓存命中链路：先用 sample.mp3 解码生成缓存，再用相同 hash 触发
// queryCacheExisted 命中（直接 emit audioBuffer，跳过解码线程）
// 注意：queryCacheExisted 读取 DmGlobal::cachePath()，而 resample 写入
// QStandardPaths::CacheLocation。本用例把 DmGlobal::cachePath 指向系统缓存，
// 使读/写两侧对齐，从而命中缓存。
// ============================================================================
TEST(AudioDataDetectorCacheTest, cacheHitSkipsDecodingAndEmitsBuffer)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path())) << "testdata/sample.mp3 missing";

    EngineTypeGuard guard(1);
    const QString hash = "decode_cache_hit_002";

    // 把 DmGlobal::cachePath 指向系统 CacheLocation，使 queryCacheExisted 读到 resample 写的文件
    const QString sysCacheRoot = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    // 向上退到 CacheLocation 根（resample 用 CacheLocation/wave，cachePath 也应是其父目录）
    DmGlobal::setCachePath(sysCacheRoot);
    removeSystemCacheFor(hash);   // 同时清理两侧（此时 dmCacheWaveDir==systemWaveDir）
    removeDmCacheFor(hash);

    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());

    // 第一阶段：解码并写缓存
    QSignalSpy spyFirst(detector.get(), &AudioDataDetector::audioBufferFromThread);
    detector->onBufferDetector(sampleMp3Path(), hash);
    ASSERT_TRUE(spyFirst.wait(20000));
    ensureThreadStopped(detector.get());
    ASSERT_GT(spyFirst.count(), 0);

    // 确认缓存已落盘
    const QString cacheFile = systemWaveDir() + QString("/%1.dat").arg(hash);
    ASSERT_TRUE(QFile::exists(cacheFile)) << "前置解码应写入缓存以供命中测试";

    // 第二阶段：用相同 hash 再次触发，这次应命中 queryCacheExisted
    // 注意：onBufferDetector 开头有 (hash == m_curHash) 直接返回的判断，
    // 需要先用不同 hash 或 onClearBufferDetector 清掉 m_curHash。
    detector->onClearBufferDetector();

    // queryCacheExisted 在 onBufferDetector 内部同步调用（主线程），命中时
    // 直接 emit audioBuffer。先验证缓存文件确实存在于 queryCacheExisted 读取的路径。
    const QString queryPath = DmGlobal::cachePath() + QString("/wave/%1.dat").arg(hash);
    ASSERT_TRUE(QFile::exists(queryPath))
        << "queryCacheExisted 读取路径应存在: " << queryPath.toStdString();

    QSignalSpy spyCached(detector.get(), &AudioDataDetector::audioBuffer);
    detector->onBufferDetector(sampleMp3Path(), hash);

    // queryCacheExisted 同步 emit audioBuffer（主线程直接调用），信号应已被捕获
    // 若同步未捕获，再用事件循环兜底等待
    if (spyCached.count() == 0) {
        spyCached.wait(3000);
        QCoreApplication::processEvents();
    }
    EXPECT_GE(spyCached.count(), 1) << "缓存命中应 emit audioBuffer";
    if (spyCached.count() > 0) {
        const QVector<float> buf = spyCached.takeFirst().at(0).value<QVector<float>>();
        EXPECT_GT(buf.size(), 0) << "缓存命中应返回非空波形数据";
    }
    // 缓存命中不应启动解码线程
    EXPECT_FALSE(detector->isRunning());
}

// ============================================================================
// engineType=0(Qt MediaPlayer) 分支：onBufferDetector 不启动解码线程
// queryCacheExisted 在 cachePath 下找不到文件且 engineType!=1 时，回退 default_music.dat
// ============================================================================
TEST(AudioDataDetectorEngineTypeTest, onBufferDetectorWithQtMediaPlayerType)
{
    EngineTypeGuard guard(0);  // Qt MediaPlayer
    // 设置独立的 dm 缓存目录，避免读到前序用例残留
    QString dmCache = QDir::temp().filePath("dm-ut-engine0-cache");
    QDir().mkpath(dmCache + "/wave");
    DmGlobal::setCachePath(dmCache);

    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    const QString hash = "engine0_hash_003";

    // engineType=0：onBufferDetector 中条件 !queryCacheExisted && type==1 为 false，
    // 不启动解码线程。queryCacheExisted 走 default_music.dat 兜底（资源文件存在则返回 true）
    QSignalSpy spy(detector.get(), &AudioDataDetector::audioBuffer);
    detector->onBufferDetector("/nonexistent.mp3", hash);

    // 不应启动线程
    EXPECT_FALSE(detector->isRunning());
    // queryCacheExisted 可能 emit audioBuffer（命中 default_music.dat）或返回 false
    spy.wait(1000);  // 容错等待，不强求信号
    SUCCEED();
}

TEST(AudioDataDetectorEngineTypeTest, onBufferDetectorWithVlcTypeNoCache)
{
    EngineTypeGuard guard(1);  // VLC/FFmpeg
    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    // engineType=1 且无缓存：会启动解码线程，但路径不存在会快速失败返回
    detector->onBufferDetector("/nonexistent.mp3", "vlc_nocache_hash_004");
    ensureThreadStopped(detector.get());
    // 线程应已结束（解码失败后 run() 返回）
    EXPECT_FALSE(detector->isRunning());
}

// ============================================================================
// stopFlag 截断：onBufferDetector 启动解码后立即 onClearBufferDetector
// 覆盖 run() 中 (m_stopFlag && curData.size()>100) 截断分支 + resample forceQuit 分支
// 注意：截断分支要求解码已产出 >100 个采样点。sample.mp3 解码足够快，
// 但 stopFlag 时机不确定；即使未命中截断分支，也验证了并发清理的安全性。
// ============================================================================
TEST(AudioDataDetectorConcurrencyTest, clearBufferDetectorStopsRunningThread)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path())) << "testdata/sample.mp3 missing";

    EngineTypeGuard guard(1);
    const QString hash = "stop_flag_hash_005";
    removeSystemCacheFor(hash);

    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());

    // 启动解码线程
    detector->onBufferDetector(sampleMp3Path(), hash);

    // 立即触发清理（设置 stopFlag）。若线程仍在运行，run() 循环中检测到
    // m_stopFlag && curData.size()>100 会走截断分支并 resample(forceQuit=true)。
    detector->onClearBufferDetector();

    // 等待线程退出（无论走正常结束还是 stopFlag 截断分支）
    ensureThreadStopped(detector.get());
    EXPECT_FALSE(detector->isRunning());

    // 验证清理后内部状态被清空：再次用空 hash 调用不崩溃
    detector->onClearBufferDetector();
    SUCCEED();
}

// ============================================================================
// 快速连续调用：测试信号槽连接和 stopFlag 重置逻辑
// ============================================================================
TEST(AudioDataDetectorConcurrencyTest, rapidBufferDetectorCallsDoNotCrash)
{
    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    // 快速连续调用（空路径不启动线程），测试 hash 比较/清理逻辑
    for (int i = 0; i < 10; ++i) {
        detector->onBufferDetector("", QString("rapid_hash_%1").arg(i));
        detector->onClearBufferDetector();
    }
    ensureThreadStopped(detector.get());
    SUCCEED();
}

// ============================================================================
// 线程安全：析构时线程仍在运行
// ============================================================================
TEST(AudioDataDetectorThreadSafetyTest, destructorHandlesRunningThread)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path()));
    EngineTypeGuard guard(1);
    const QString hash = "dtor_running_hash_006";
    removeSystemCacheFor(hash);

    // 在单独 scope 中创建并启动线程，scope 结束时析构等待线程退出
    {
        AudioDataDetector *detector = new AudioDataDetector();
        detector->onBufferDetector(sampleMp3Path(), hash);
        // 立即析构：~AudioDataDetector 设置 m_stopFlag=true 并 while(isRunning()) 等待
        // 覆盖析构函数的 stopFlag 等待循环
        delete detector;
    }
    SUCCEED();
}

// ============================================================================
// 信号元对象验证
// ============================================================================
TEST(AudioDataDetectorSignalTest, audioBufferSignalExists)
{
    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    const QMetaObject *mo = detector->metaObject();
    EXPECT_GE(mo->indexOfSignal("audioBuffer(QVector<float>,QString)"), 0);
}

TEST(AudioDataDetectorSignalTest, audioBufferFromThreadSignalExists)
{
    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    const QMetaObject *mo = detector->metaObject();
    EXPECT_GE(mo->indexOfSignal("audioBufferFromThread(QVector<float>,QString)"), 0);
}

// ============================================================================
// 信号连接验证：构造时建立 audioBufferFromThread → audioBuffer 的 QueuedConnection
// 通过实际 sample.mp3 触发验证两个信号都被 emit（解码链路 + queued 转发）
// ============================================================================
TEST(AudioDataDetectorConnectionTest, queuedConnectionForwardsSignalOnRealDecode)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path()));
    EngineTypeGuard guard(1);
    const QString hash = "conn_queued_hash_007";
    removeSystemCacheFor(hash);

    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    QSignalSpy spyThread(detector.get(), &AudioDataDetector::audioBufferFromThread);
    QSignalSpy spyBuf(detector.get(), &AudioDataDetector::audioBuffer);

    detector->onBufferDetector(sampleMp3Path(), hash);
    ASSERT_TRUE(spyThread.wait(20000));
    ensureThreadStopped(detector.get());

    // 处理 queued 事件
    QCoreApplication::processEvents();
    if (spyBuf.count() == 0) {
        QTest::qWait(100);
        QCoreApplication::processEvents();
    }
    // 验证 queued connection 把 worker 信号转发到了主线程槽
    EXPECT_GE(spyBuf.count(), 1);
}

// ============================================================================
// 边界测试：特殊字符路径/Unicode 路径（不存在，走解码失败分支）
// ============================================================================
TEST(AudioDataDetectorBoundaryTest, onBufferDetectorWithSpecialCharacterPath)
{
    EngineTypeGuard guard(1);
    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    // 特殊字符路径（不存在），ffmpeg 打开失败后清理退出
    detector->onBufferDetector("/path/with spaces/audio.mp3", "hash_special_chars");
    ensureThreadStopped(detector.get());
    SUCCEED();
}

TEST(AudioDataDetectorBoundaryTest, onBufferDetectorWithUnicodePath)
{
    EngineTypeGuard guard(1);
    std::unique_ptr<AudioDataDetector> detector(new AudioDataDetector());
    detector->onBufferDetector("/音乐/歌曲/audio.mp3", "hash_unicode_path");
    ensureThreadStopped(detector.get());
    SUCCEED();
}

// ============================================================================
// 缓存目录测试：设置测试用缓存目录
// ============================================================================
TEST(AudioDataDetectorCachePathTest, cacheDirectoryCanBeSet)
{
    QString tmp = QDir::temp().filePath("dm-ut-cachepath-test");
    DmGlobal::setCachePath(tmp);
    EXPECT_EQ(DmGlobal::cachePath(), tmp);
    // 空字符串不应覆盖
    DmGlobal::setCachePath("");
    EXPECT_EQ(DmGlobal::cachePath(), tmp);
}
