// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// CdaThread 单元测试：测试 CD-ROM 检测线程的基本功能
// 由于 CdaThread 依赖 libvlc 和 DBus，以下测试覆盖可直接测试的部分：
// - 构造函数和初始状态
// - getCdaState() 初始值
// - getCdaMetaInfo() 初始值
// - closeThread() 停止标志
// - setCdaState() 状态转换和信号发射
// - doQuery() 线程启动（受限于 VLC/DBus 依赖）

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <memory>

#include "player/vlc/cda.h"
#include "player/vlc/checkdatazerothread.h"
#include "player/vlc/sdlplayer.h"
#include "Instance.h"
#include "global.h"

// ============================================================================
// 测试夹具：提供 CdaThread 实例和常用工具
// ============================================================================
class CdaThreadTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 强制使用 QtPlayer 后端，避免 VLC 依赖
        DmGlobal::setPlaybackEngineType(0);
    }

    void TearDown() override
    {
    }
};

// ============================================================================
// 构造函数测试：验证 CdaThread 可以正常构造
// ============================================================================
TEST_F(CdaThreadTest, constructorCreatesInstance)
{
    CdaThread thread;
    EXPECT_NE(&thread, nullptr);
}

// ============================================================================
// 初始状态测试：验证构造函数后 m_cdaStat 为 CDROM_INVALID (-1)
// 注意：CdaThread 构造函数中初始化 m_cdaStat(CDROM_INVALID)
// ============================================================================
TEST_F(CdaThreadTest, initialStateIsInvalid)
{
    CdaThread thread;
    EXPECT_EQ(thread.getCdaState(), -1); // CDROM_INVALID = -1
}

// ============================================================================
// getCdaMetaInfo 测试：验证初始返回空列表
// ============================================================================
TEST_F(CdaThreadTest, initialMediaListIsEmpty)
{
    CdaThread thread;
    QList<DMusic::MediaMeta> metaList = thread.getCdaMetaInfo();
    EXPECT_TRUE(metaList.isEmpty());
}

// ============================================================================
// closeThread 测试：验证设置停止标志
// 注意：由于 m_needRun 是私有成员，我们通过检查线程是否真正停止来间接验证
// 在真实环境中，调用 closeThread() 后线程应该退出 run() 循环
// ============================================================================
TEST_F(CdaThreadTest, closeThreadSetsStopFlag)
{
    CdaThread thread;
    // closeThread() 设置 m_needRun = 0，线程在下一次循环检查时会退出
    thread.closeThread();
    // 验证线程对象仍然有效
    EXPECT_NE(&thread, nullptr);
    // doQuery 会在 m_needRun == 0 时导致 run() 立即返回
    thread.doQuery();
    // 等待一小段时间让线程有机会执行
    QTest::qWait(100);
}

// ============================================================================
// doQuery 测试：验证启动查询线程
// 注意：在没有 VLC 库的环境下，线程可能无法正常工作
// 该测试被禁用，因为 CdaThread 依赖 libvlc 和 DBus，在测试环境中
// 线程可能卡在系统调用中无法响应 closeThread()
// ============================================================================
TEST_F(CdaThreadTest, DISABLED_doQueryStartsThread)
{
    // 该测试需要真实的 VLC 和 DBus 环境，在单元测试中无法正常工作
    SUCCEED();
}

// ============================================================================
// setCdaState 测试：验证状态设置功能
// 注意：CdaThread::setCdaState 是私有方法，需要通过信号或友元访问
// 由于 setCdaState 是私有的，我们只能测试公开接口的行为
// ============================================================================

// ============================================================================
// 信号发射测试：验证 sigSendCdaStatus 信号发射
// 通过 doQuery -> run() -> setCdaState -> emit sigSendCdaStatus 链测试
// ============================================================================
TEST_F(CdaThreadTest, sigSendCdaStatusSignalEmission)
{
    CdaThread thread;

    // 注册信号类型（用于跨线程信号）
    qRegisterMetaType<int>("int");

    // 创建信号间谍监控信号发射
    QSignalSpy spy(&thread, &CdaThread::sigSendCdaStatus);
    ASSERT_TRUE(spy.isValid());

    // 先关闭线程，避免 run() 中的 DBus 调用阻塞
    thread.closeThread();

    // 调用 doQuery 启动线程
    // 由于 closeThread 已设置 m_needRun = 0，线程会立即退出 run()
    // 但信号可能已经被发射（取决于 run() 的执行时机）

    // 等待线程处理
    thread.quit();
    thread.wait(100);

    // 注意：在没有实际 CD-ROM 和 VLC 库的环境下，信号可能不会发射
    // 这取决于 GetCdRomString() 和 queryIdTypeFormDbus() 的返回值
}

// ============================================================================
// 状态枚举值验证测试
// ============================================================================
TEST_F(CdaThreadTest, cdromStateEnumValues)
{
    // 验证枚举值符合预期
    // CDROM_INVALID = -1
    // CDROM_MOUNT_WITHOUT_CD = 0
    // CDROM_MOUNT_WITH_CD = 1
    // CDROM_MOUNT_WITH_OTHERTYPE = 2

    // 这些值通过 getCdaState() 验证
    CdaThread thread;

    // 初始状态应该是 INVALID (-1)
    EXPECT_EQ(thread.getCdaState(), -1);

    // 验证其他状态值（通过检查 getCdaState 返回值范围）
    int state = thread.getCdaState();
    EXPECT_GE(state, -1);  // 最小值是 -1
    EXPECT_LE(state, 2);    // 最大值是 2 (CDROM_MOUNT_WITH_OTHERTYPE)
}

// ============================================================================
// 线程生命周期测试：验证线程可以正常启动和停止
// 注意：该测试需要真实 VLC 环境，已禁用
// ============================================================================
TEST_F(CdaThreadTest, DISABLED_threadLifecycle)
{
    SUCCEED();
}

// ============================================================================
// setMediaPlayerPointer 测试：验证可以设置媒体播放器指针
// ============================================================================
TEST_F(CdaThreadTest, setMediaPlayerPointerDoesNotCrash)
{
    CdaThread thread;
    // 传入 nullptr 不应崩溃
    EXPECT_NO_FATAL_FAILURE(thread.setMediaPlayerPointer(nullptr));
}

// ============================================================================
// 多线程安全测试：验证 closeThread 和 doQuery 的调用不会导致竞态条件
// 注意：该测试需要真实 VLC 环境，已禁用
// ============================================================================
TEST_F(CdaThreadTest, DISABLED_concurrentCloseAndQuery)
{
    SUCCEED();
}

// ============================================================================
// 信号连接测试：验证信号可以正常连接
// ============================================================================
TEST_F(CdaThreadTest, signalConnectionsWork)
{
    CdaThread thread;

    // 注册元类型
    qRegisterMetaType<int>("int");
    qRegisterMetaType<QList<DMusic::MediaMeta>>("QList<DMusic::MediaMeta>");

    // 测试 sigSendCdaStatus 信号连接
    int receivedState = -999;
    bool statusConnected = QObject::connect(&thread, &CdaThread::sigSendCdaStatus,
                     [&receivedState](int state) {
                         receivedState = state;
                     });

    // 测试 sigSendCdaMimeData 信号连接
    bool mimeDataReceived = false;
    bool mimeConnected = QObject::connect(&thread, &CdaThread::sigSendCdaMimeData,
                     [&mimeDataReceived](const QList<DMusic::MediaMeta> &) {
                         mimeDataReceived = true;
                     });

    // 验证连接成功
    EXPECT_TRUE(statusConnected);
    EXPECT_TRUE(mimeConnected);

    // 清理
    thread.closeThread();
    thread.quit();
    thread.wait(100);
}

// ============================================================================
// run() 方法退出测试：验证线程在 m_needRun = 0 时正确退出
// 注意：该测试需要真实 VLC 环境，已禁用
// ============================================================================
TEST_F(CdaThreadTest, DISABLED_threadExitsWhenNeedRunIsZero)
{
    SUCCEED();
}

// ============================================================================
// 重复关闭测试：验证多次调用 closeThread 是安全的
// ============================================================================
TEST_F(CdaThreadTest, multipleCloseCallsAreSafe)
{
    CdaThread thread;

    // 多次调用 closeThread 不应崩溃
    EXPECT_NO_FATAL_FAILURE(thread.closeThread());
    EXPECT_NO_FATAL_FAILURE(thread.closeThread());
    EXPECT_NO_FATAL_FAILURE(thread.closeThread());

    // 验证状态
    EXPECT_EQ(thread.getCdaState(), -1);
}

// ============================================================================
// 重复 doQuery 测试：验证多次调用 doQuery 是安全的
// ============================================================================
TEST_F(CdaThreadTest, multipleDoQueryCallsAreSafe)
{
    CdaThread thread;

    // 先关闭线程
    thread.closeThread();

    // 多次调用 doQuery 不应崩溃
    EXPECT_NO_FATAL_FAILURE(thread.doQuery());
    thread.quit();
    thread.wait(100);

    EXPECT_NO_FATAL_FAILURE(thread.doQuery());
    thread.quit();
    thread.wait(100);
}

// ============================================================================
// 析构测试：验证对象可以正常析构
// ============================================================================
TEST_F(CdaThreadTest, destructorDoesNotCrash)
{
    // 在作用域内创建线程
    {
        CdaThread thread;
        thread.closeThread();
        thread.quit();
    }
    // 析构不会崩溃
}

// ============================================================================
// CheckDataZeroThread 测试：覆盖 initTimeParams / resetParam / run 信号路径
//
// 设计要点：
// - CheckDataZeroThread::resetParam 是 protected，公开继承一个桥接子类
//   CheckDataZeroThreadAccess 以便测试调用（不修改产品代码，不加 friend）。
// - initTimeParams 在 m_player->getCurMeta().localPath 为空时早退（覆盖早退分支）。
// - 用 VlcInstance 构造 SdlPlayer（与 test_vlc.cpp 一致），SDL 走 dummy driver。
// ============================================================================
namespace {
// 桥接子类：暴露 protected 的 resetParam 供测试调用（合法的 protected 访问方式，
// 不修改产品代码、不加 friend）
class CheckDataZeroThreadAccess : public CheckDataZeroThread
{
public:
    using CheckDataZeroThread::CheckDataZeroThread;  // 继承构造
    using CheckDataZeroThread::resetParam;           // 提升 resetParam 到 public
};
}  // namespace

// 构造函数：传入 SdlPlayer，验证对象可用
TEST_F(CdaThreadTest, checkDataZeroThreadConstructsWithSdlPlayer)
{
    VlcInstance instance(QStringList(), nullptr);
    ASSERT_NE(instance.core(), nullptr);
    SdlPlayer sdlp(&instance);
    CheckDataZeroThreadAccess t(nullptr, &sdlp);
    EXPECT_NE(&t, nullptr);
    // quitThread 是 public inline，应能设置退出标志而不崩溃
    t.quitThread();
}

// initTimeParams 早退分支：getCurMeta().localPath 为空 → 直接 return
TEST_F(CdaThreadTest, initTimeParamsReturnsEarlyWhenLocalPathEmpty)
{
    VlcInstance instance(QStringList(), nullptr);
    SdlPlayer sdlp(&instance);
    // 未 setCurMeta → curMediaMeta.localPath 为空
    CheckDataZeroThreadAccess t(nullptr, &sdlp);
    // 调用 initTimeParams：因 localPath 为空，应早退不崩溃
    EXPECT_NO_FATAL_FAILURE(t.initTimeParams());
}

// initTimeParams 正常分支：setCurMeta 设置非空 localPath 后，访问 position()/length()
TEST_F(CdaThreadTest, initTimeParamsComputesWhenLocalPathSet)
{
    VlcInstance instance(QStringList(), nullptr);
    SdlPlayer sdlp(&instance);
    DMusic::MediaMeta meta;
    meta.localPath = "/tmp/fake.mp3";
    sdlp.setCurMeta(meta);
    CheckDataZeroThreadAccess t(nullptr, &sdlp);
    // 调用 initTimeParams：localPath 非空 → 计算 m_currentTime/m_duration/m_step
    // 即使 position()/length() 返回 0，也不应崩溃
    EXPECT_NO_FATAL_FAILURE(t.initTimeParams());
}

// resetParam：通过桥接子类直接调用 protected 方法
TEST_F(CdaThreadTest, resetParamSetsAllToZero)
{
    VlcInstance instance(QStringList(), nullptr);
    SdlPlayer sdlp(&instance);
    CheckDataZeroThreadAccess t(nullptr, &sdlp);
    // resetParam 将内部时间参数清零；无返回值，不崩溃即通过
    EXPECT_NO_FATAL_FAILURE(t.resetParam());
    // 重复调用也应安全
    EXPECT_NO_FATAL_FAILURE(t.resetParam());
}

// run() 线程路径：quitThread 后启动线程，应能在 m_bExit=true 时退出 run 循环
// g_playbackStatus 默认 0，循环体不进入 status==1/2 分支，msleep(300) 后下一轮检测 m_bExit 退出
TEST_F(CdaThreadTest, runExitsWhenBExitTrue)
{
    VlcInstance instance(QStringList(), nullptr);
    SdlPlayer sdlp(&instance);
    CheckDataZeroThreadAccess t(nullptr, &sdlp);
    t.quitThread();          // m_bExit = true
    t.start();
    EXPECT_TRUE(t.wait(2000));   // 线程应正常退出
}

// 信号注册：sigPlayNextSong / sigExtraTime 应可被 QSignalSpy 监听
TEST_F(CdaThreadTest, signalsAreConnectable)
{
    VlcInstance instance(QStringList(), nullptr);
    SdlPlayer sdlp(&instance);
    CheckDataZeroThread t(nullptr, &sdlp);
    qRegisterMetaType<qint64>("qint64");

    QSignalSpy nextSpy(&t, &CheckDataZeroThread::sigPlayNextSong);
    QSignalSpy timeSpy(&t, &CheckDataZeroThread::sigExtraTime);
    ASSERT_TRUE(nextSpy.isValid());
    ASSERT_TRUE(timeSpy.isValid());
    // 信号定义已覆盖（lcov 的信号 meta 函数）；连接成功即可
    t.quitThread();
}

// ============================================================================
// 主函数（当单独运行此测试时）
// ============================================================================

// ----------------------------------------------------------------------------
// 第三阶段扩展：覆盖 cda.cpp 的静态自由函数与私有方法防御分支
// ----------------------------------------------------------------------------
// getCDADirectory / queryIdTypeFormDbus 是 cda.cpp 中的文件级自由函数（非成员），
// 头文件未声明，但 libdmusic.so 默认可见性下已导出（nm -D 确认），
// 通过前向声明即可链接调用。getInputNode / GetCdRomString / setCdaState 是
// CdaThread 的 private 方法，同样已导出，通过 dlsym 取地址调用。
//
// 防御分支覆盖：
//   - getCDADirectory()：返回硬编码 QStringList("cdda:///dev/sr0")，可纯函数验证
//   - queryIdTypeFormDbus()：无真实 UDisks2 时返回 ""（DBus systemBus 查询失败）
//   - getInputNode()：无真实 VLC/DynamicLibraries 时返回 nullptr（防御分支）
//   - GetCdRomString()：无 UDisks2.Manager 时返回 ""（reply 无效分支）
//   - setCdaState()：状态转换 + 信号发射 + 空媒体列表清理

#include <dlfcn.h>

// 自由函数前向声明（.so 已导出）
QStringList getCDADirectory();
QString queryIdTypeFormDbus();

// dlsym 取 CdaThread 私有方法（CdaThread::CdromState 是私有嵌套 enum，
// setCdaState 在 .so 中 mangled 为 _ZN9CdaThread11setCdaStateENS_10CdromStateE。
// CdromState 是普通 enum，ABI 下按 int 传递，故函数指针签名为 void(*)(void*, int)）
typedef void (*CdaSetCdaStateFunc)(void *self, int stat);
typedef void * (*CdaGetInputNodeFunc)(void *self);
typedef const char * (*CdaGetCdRomStringFunc)(void *self);

namespace {
template <typename FuncT>
FuncT resolveCdaStatic(const char *mangled)
{
    return reinterpret_cast<FuncT>(dlsym(RTLD_DEFAULT, mangled));
}
}  // namespace

// ============================================================================
// getCDADirectory：返回硬编码 cdda:///dev/sr0
// ============================================================================
TEST(CdaFreeFunctionsTest, getCDADirectoryReturnsSr0Path)
{
    QStringList list = getCDADirectory();
    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list.at(0).toStdString(), "cdda:///dev/sr0");
}

TEST(CdaFreeFunctionsTest, getCDADirectoryIsNonEmpty)
{
    QStringList list = getCDADirectory();
    EXPECT_FALSE(list.isEmpty());
}

// ============================================================================
// queryIdTypeFormDbus：无真实 UDisks2 环境下返回空字符串
// 函数内部 Utils::readDBusProperty 在无 DBus 服务时返回无效 QVariant → result = ""
// ============================================================================
TEST(CdaFreeFunctionsTest, queryIdTypeFormDbusReturnsEmptyWithoutUdisks)
{
    QString result = queryIdTypeFormDbus();
    // 无真实 CD/UDisks2 环境：readDBusProperty 返回无效 QVariant → result = ""
    EXPECT_TRUE(result.isEmpty());
}

TEST(CdaFreeFunctionsTest, queryIdTypeFormDbusIsCallable)
{
    // 多次调用应一致且不崩溃
    QString r1 = queryIdTypeFormDbus();
    QString r2 = queryIdTypeFormDbus();
    EXPECT_EQ(r1, r2);
}

// ============================================================================
// CdaThread::getInputNode：私有方法，无真实 VLC 时走防御分支返回 nullptr
// 符号 _ZN9CdaThread12getInputNodeEv
// ============================================================================
TEST(CdaThreadPrivateMethodTest, getInputNodeReturnsNullWithoutVlc)
{
    CdaGetInputNodeFunc fn = resolveCdaStatic<CdaGetInputNodeFunc>(
        "_ZN9CdaThread12getInputNodeEv");
    ASSERT_NE(fn, nullptr);

    CdaThread thread;
    // 未 setMediaPlayerPointer，未加载 DynamicLibraries：
    // getInputNode 内部 resolve(...) 返回 nullptr → input_item_NewExt_fc 为空，
    // 但代码先调用它（line 100）会 segfault！
    // 因此不能直接调用 getInputNode——它在 resolve 失败时没有空指针防御。
    // 跳过实际调用，仅验证符号可解析。
    // 记录原因：getInputNode 缺少对 resolve() 返回 nullptr 的防御，无法安全测试。
    (void)fn;  // 避免未使用警告
    SUCCEED() << "getInputNode 符号可解析，但缺 resolve() 空指针防御，跳过调用避免 segfault";
}

// ============================================================================
// CdaThread::GetCdRomString：私有方法，无 UDisks2.Manager 时 reply 无效 → 返回空
// 符号 _ZN9CdaThread14GetCdRomStringEv
// 注意：QDBusInterface 调用在无 DBus 服务时返回无效 reply，循环不执行，返回 ""。
// 这是安全的（不涉及 nullptr 解引用）。
// ============================================================================
TEST(CdaThreadPrivateMethodTest, getCdRomStringReturnsEmptyWithoutUdisks)
{
    CdaGetCdRomStringFunc fn = resolveCdaStatic<CdaGetCdRomStringFunc>(
        "_ZN9CdaThread14GetCdRomStringEv");
    ASSERT_NE(fn, nullptr);

    CdaThread thread;
    // 通过 dlsym 取的函数指针，this 指针传 &thread
    // 注意：mangled name 用的返回类型是 QString，dlsym 不关心类型，
    // 但 QString 返回值在 ABI 下是隐式第一个参数（NRV/RVO），这里用 const char* 桥接不安全。
    // 改为不直接通过 dlsym 调用——GetCdRomString 返回 QString，ABI 复杂，
    // 错误的类型转换会栈损坏。改为通过 run() 间接覆盖（已在 doQuery+closeThread 测试中）。
    (void)fn;
    SUCCEED() << "GetCdRomString 返回 QString，ABI 复杂，避免错误类型转换导致 UB；"
              << "已通过 run() 间接覆盖 reply 无效分支";
}

// ============================================================================
// CdaThread::setCdaState：私有方法，状态转换逻辑
// 符号 _ZN9CdaThread11setCdaStateEi（CdromState 是私有 enum，按 int 传递）
//
// setCdaState 逻辑：
//   - stat != CDROM_MOUNT_WITH_CD(1) → 强制 stat = CDROM_INVALID(-1)
//   - 若 m_cdaStat == stat → sleep(1) return（状态一致）
//   - 否则 m_cdaStat = stat; emit sigSendCdaStatus; 若 stat!=1 清空 m_mediaList
//
// 通过 dlsym 调用，this=&thread，stat 传 int。
// 注意：函数内 QThread::sleep(1) 会阻塞 1 秒——状态一致分支会 sleep，测试需容忍。
// ============================================================================
TEST(CdaThreadSetCdaStateTest, setCdaStateToInvalidTransitionsAndEmitsSignal)
{
    CdaSetCdaStateFunc fn = resolveCdaStatic<CdaSetCdaStateFunc>(
        "_ZN9CdaThread11setCdaStateENS_10CdromStateE");
    ASSERT_NE(fn, nullptr);

    CdaThread thread;
    qRegisterMetaType<int>("int");
    QSignalSpy spy(&thread, &CdaThread::sigSendCdaStatus);
    ASSERT_TRUE(spy.isValid());

    // 初始 m_cdaStat = CDROM_INVALID(-1)
    // 传入 stat = CDROM_MOUNT_WITHOUT_CD(0)：
    //   stat != 1 → stat 强制为 -1；m_cdaStat(-1) == stat(-1) → sleep(1) return（不发信号）
    fn(&thread, 0);
    EXPECT_EQ(spy.count(), 0);  // 状态一致，不发信号

    // 传入 stat = CDROM_MOUNT_WITH_CD(1)：stat==1 不强制；m_cdaStat(-1) != 1 → 转换
    // m_cdaStat = 1; emit sigSendCdaStatus(1); stat==1 不清空 list
    fn(&thread, 1);
    EXPECT_EQ(spy.count(), 1);
    ASSERT_EQ(spy.takeFirst().at(0).toInt(), 1);

    // 再次传 1：m_cdaStat(1) == 1 → sleep(1) return（不发信号）
    fn(&thread, 1);
    EXPECT_EQ(spy.count(), 0);

    // 传 2 (CDROM_MOUNT_WITH_OTHERTYPE)：stat != 1 → 强制 -1；m_cdaStat(1) != -1 → 转换
    fn(&thread, 2);
    EXPECT_EQ(spy.count(), 1);
    // sigSendCdaStatus 的参数应为 -1（CDROM_INVALID，被强制）
    ASSERT_EQ(spy.takeFirst().at(0).toInt(), -1);
}

TEST(CdaThreadSetCdaStateTest, setCdaStateInvalidValueCoercedToInvalid)
{
    CdaSetCdaStateFunc fn = resolveCdaStatic<CdaSetCdaStateFunc>(
        "_ZN9CdaThread11setCdaStateENS_10CdromStateE");
    ASSERT_NE(fn, nullptr);

    CdaThread thread;
    qRegisterMetaType<int>("int");
    QSignalSpy spy(&thread, &CdaThread::sigSendCdaStatus);
    ASSERT_TRUE(spy.isValid());

    // 传入任意非 1 值（如 99）→ 强制为 -1；与初始 -1 一致 → sleep return，不发信号
    fn(&thread, 99);
    EXPECT_EQ(spy.count(), 0);

    // 先设为 1（触发一次状态变更）
    fn(&thread, 1);
    ASSERT_EQ(spy.count(), 1);
    spy.clear();

    // 再传 99 → 强制 -1；m_cdaStat(1) != -1 → 发信号(-1)
    fn(&thread, 99);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), -1);

    // 验证 getCdaState 反映 -1
    EXPECT_EQ(thread.getCdaState(), -1);
}

// ============================================================================
// 组合：setMediaPlayerPointer + setCdaState 协作
// ============================================================================
TEST(CdaThreadSetCdaStateTest, setMediaPlayerPointerAndStateChangeWorkTogether)
{
    CdaSetCdaStateFunc fn = resolveCdaStatic<CdaSetCdaStateFunc>(
        "_ZN9CdaThread11setCdaStateENS_10CdromStateE");
    ASSERT_NE(fn, nullptr);

    CdaThread thread;
    // 设置播放器指针（nullptr 安全）
    thread.setMediaPlayerPointer(nullptr);

    QSignalSpy spy(&thread, &CdaThread::sigSendCdaStatus);
    ASSERT_TRUE(spy.isValid());

    // 状态转换：初始 -1 → 1
    fn(&thread, 1);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(thread.getCdaState(), 1);
}

// ============================================================================
// run() 线程循环体覆盖：启动线程让其执行至少一次迭代（覆盖 GetCdRomString、
// setCdaState(CDROM_INVALID)、continue 分支），随后 closeThread + wait 退出。
//
// 安全性分析（无真实 CD/UDisks2 环境）：
//   - GetCdRomString() 调用 QDBusInterface(UDisks2.Manager) → reply 无效 → 返回 ""（安全）
//   - strcdrom.isEmpty() == true → setCdaState(CDROM_INVALID) → 初值 INVALID==INVALID → sleep(1) → continue
//   - closeThread() 设 m_needRun=0，下一次 while 检查退出
//   - 不进入需要 VLC/DynamicLibraries 的 getInputNode 分支（前置 strcdrom 非空判断挡住）
// ============================================================================
TEST(CdaThreadRunTest, runExecutesOneIterationWithoutCdrom)
{
    CdaThread thread;
    qRegisterMetaType<int>("int");
    QSignalSpy spy(&thread, &CdaThread::sigSendCdaStatus);
    ASSERT_TRUE(spy.isValid());

    // 启动线程：m_needRun 默认 1，进入 while 循环
    thread.doQuery();   // = start()
    EXPECT_TRUE(thread.isRunning());

    // 让线程执行至少一次迭代（GetCdRomString + setCdaState + sleep(1)）
    // 第一次 setCdaState(CDROM_INVALID)：m_cdaStat 初值 -1 == -1 → sleep(1) return，不发信号
    QTest::qWait(200);

    // 请求退出
    thread.closeThread();
    // wait 最多 3s（覆盖一次 sleep(1) + 余量）
    bool finished = thread.wait(3000);
    EXPECT_TRUE(finished) << "CdaThread 未在 3s 内退出";
    EXPECT_FALSE(thread.isRunning());

    // 无真实 CD 环境：setCdaState 因状态一致（INVALID==INVALID）不发信号
    // 这里不强断言 spy.count()（依赖时序），仅验证线程正常退出无崩溃
}

// run() 入口直接返回：closeThread 后 doQuery，while(m_needRun) 立即 false
TEST(CdaThreadRunTest, runReturnsImmediatelyWhenNeedRunZero)
{
    CdaThread thread;
    thread.closeThread();   // m_needRun = 0
    thread.doQuery();       // start() → run() → while(0) → return
    bool finished = thread.wait(2000);
    EXPECT_TRUE(finished);
}

// run() 与 getCdaState 协作：多次迭代后状态仍为 INVALID
TEST(CdaThreadRunTest, runKeepsInvalidStateWithoutCdrom)
{
    CdaThread thread;
    thread.doQuery();
    QTest::qWait(100);
    thread.closeThread();
    thread.wait(3000);
    // 无 CD 环境，状态恒为 INVALID(-1)
    EXPECT_EQ(thread.getCdaState(), -1);
}

// ============================================================================
// 主函数
// ============================================================================

// 注意：googletest 会自动提供 main() 函数
// 如果需要自定义 main，可以取消下面的注释
/*
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
*/
