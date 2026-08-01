// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QQmlApplicationEngine>
#include <QScopedPointer>
#include <QQmlContext>
#include <QStandardPaths>
#include <QIcon>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#include <QSharedMemory>
#endif

#include <DLog>
#include <DGuiApplicationHelper>
#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QWindow>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef Q_OS_LINUX
#include <signal.h>
#include <unistd.h>
#endif

#include "config.h"

#include "effect/shaderimageview.h"
#include "effect/shaderdataview.h"
#include "presenter.h"
#include "util/eventsfilter.h"
#include "util/shortcut.h"
#include "util/dbusadpator.h"
#include "util/log.h"

DCORE_USE_NAMESPACE;
DGUI_USE_NAMESPACE;

QScopedPointer<Presenter, QScopedPointerPodDeleter> presenter;

#ifdef Q_OS_LINUX
void sig_term_handler(int signum, siginfo_t *info, void *ptr)
{
    qDebug() << "SIGTERM received.";
    presenter->saveDataToDB();
    exit(1);
}
#endif

// 此文件是QML应用的启动文件，一般无需修改
int main(int argc, char *argv[])
{
    qCDebug(dmMusic) << "main start";
#ifdef Q_OS_LINUX
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        qCDebug(dmMusic) << "XDG_CURRENT_DESKTOP is not deepin";

        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
    setenv("PULSE_PROP_media.role", "music", 1);
#endif

#ifdef Q_OS_WIN
    // Auto-set environment variables so deepin-music.exe can run directly
    // without needing a launcher batch file
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    QString appDir = QFileInfo(QString::fromWCharArray(exePath)).absolutePath();
    if (qgetenv("QT_PLUGIN_PATH").isEmpty()) {
        qputenv("QT_PLUGIN_PATH", (appDir + "/plugins").toUtf8());
    }
    if (qgetenv("QT_QPA_PLATFORM_PLUGIN_PATH").isEmpty()) {
        qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", (appDir + "/plugins/platforms").toUtf8());
    }
    if (qgetenv("QML2_IMPORT_PATH").isEmpty()) {
        qputenv("QML2_IMPORT_PATH", (appDir + "/qml").toUtf8());
    }
    if (qgetenv("QT_QML_IMPORT_PATH").isEmpty()) {
        qputenv("QT_QML_IMPORT_PATH", (appDir + "/qml").toUtf8());
    }
    if (qgetenv("DSG_DATA_DIRS").isEmpty()) {
        qputenv("DSG_DATA_DIRS", (appDir + "/dsg").toUtf8());
    }
    // Add app directory to PATH for DLL resolution
    QString currentPath = QString::fromLocal8Bit(qgetenv("PATH"));
    if (!currentPath.contains(appDir)) {
        qputenv("PATH", (appDir + ";" + currentPath).toUtf8());
    }
#endif

    DmGlobal::checkWaylandMode();

    QSurfaceFormat format;
#ifdef Q_OS_LINUX
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setVersion(3, 2);
#endif
    format.setDefaultFormat(format);
    // 1.可以使用自己创建的 QGuiApplication 对象；
    // 2.可以在创建 QGuiApplication 之前为程序设置一些属性（如使用
    //   QCoreApplication::setAttribute 禁用屏幕缩放）；
    // 3.可以添加一些在 QGuiApplication 构造过程中才需要的环境变量；

    // TODO: 无 XDG_CURRENT_DESKTOP 变量时，将不会加载 deepin platformtheme 插件，会导致
    // 查找图标的接口无法调用 qt5integration 提供的插件，后续应当把图标查找相关的功能移到 dtkgui
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")) {
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }
    qputenv("D_POPUP_MODE", "embed");
#ifdef Q_OS_WIN
    qputenv("D_DTK_DISABLE_INWINDOWBLUR", "1");
#endif

    QGuiApplication *app = new QGuiApplication(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app->setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    app->setOrganizationName("deepin");
    app->setApplicationName("deepin-music");
    // Version Time
    app->setApplicationVersion(VERSION);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin music player.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "Music file path");
    parser.process(*app);

    // handle open file
    QStringList OpenFilePaths = parser.positionalArguments();
    if (!OpenFilePaths.isEmpty()) {
        qCDebug(dmMusic) << "OpenFilePaths: " << OpenFilePaths;
        QStringList strList;
        for (QString str : OpenFilePaths) {
            QUrl url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(str));
            strList.append(url.toLocalFile().isEmpty() ? str : url.toLocalFile());
        }
        OpenFilePaths = strList;
    }

#ifdef Q_OS_LINUX
    if (!DGuiApplicationHelper::setSingleInstance("deepinmusic")) {
        qCDebug(dmMusic) << "another deepin music has started";
        QDBusInterface speechbus("org.mpris.MediaPlayer2.DeepinMusic",
                                 "/org/mpris/speech",
                                 "com.deepin.speech",
                                 QDBusConnection::sessionBus());

        if (speechbus.isValid()) {
            qCDebug(dmMusic) << "another deepin music has started, call OpenUris";
            QVariant mediaMeta;
            mediaMeta.setValue(OpenFilePaths);
            speechbus.asyncCall("OpenUris", OpenFilePaths);
        }

        QDBusInterface iface("org.mpris.MediaPlayer2.DeepinMusic",
                             "/org/mpris/MediaPlayer2",
                             "org.mpris.MediaPlayer2",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            qCDebug(dmMusic) << "another deepin music has started, call Raise";
            iface.asyncCall("Raise");
        }
        qCDebug(dmMusic) << "another deepin music has started, return";
        return 0;
    }
#endif

#ifdef Q_OS_WIN
    // Windows 单实例实现：使用命名互斥量
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"deepin-music-single-instance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        qCDebug(dmMusic) << "another deepin music has started on Windows";
        // 尝试激活现有窗口
        HWND existingWindow = FindWindowW(NULL, L"Music");
        if (existingWindow) {
            SetForegroundWindow(existingWindow);
            ShowWindow(existingWindow, SW_RESTORE);
        }
        return 0;
    }
#endif

    DmGlobal::initPlaybackEngineType();
    app->setQuitOnLastWindowClosed(false);
    DGuiApplicationHelper::loadTranslator();

    // 请在此处注册QML中的C++类型
    qmlRegisterType<ShaderImageView>("audio.image", 1, 0, "View_image");
    qmlRegisterType<ShaderDataView>("audio.image", 1, 0, "View_data");
    qmlRegisterType<DmGlobal>("audio.global", 1, 0, "DmGlobal");

    DmGlobal::setAppName(QObject::tr("Music"));

    QQmlApplicationEngine engine;
    // 请在此处注册需要导入到QML中的C++类型
    // 例如： engine.rootContext()->setContextProperty("Utils", new Utils);
    presenter.reset(new Presenter(QObject::tr("Unknown album"), QObject::tr("Unknown artist"), app));

    EventsFilter eventsFilter(presenter.data());
    Shortcut shortcut(presenter.data());

    ApplicationAdaptor adaptor(presenter.data());
    QDBusConnection::sessionBus().registerObject("/org/mpris/speech", "com.deepin.speech", &adaptor, QDBusConnection::RegisterOption::ExportAllSlots);

    presenter->setMprisPlayer("DeepinMusic", "deepin-music", "Deepin Music Player");
    engine.rootContext()->setContextProperty("Presenter", presenter.data());
    engine.rootContext()->setContextProperty("EventsFilter", &eventsFilter);
    engine.rootContext()->setContextProperty("ShortcutDialg", &shortcut);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    engine.rootObjects()[0]->installEventFilter(&eventsFilter);
#ifdef Q_OS_WIN
    if (auto *window = qobject_cast<QWindow*>(engine.rootObjects()[0])) {
        window->setIcon(QIcon(":/dsg/img/deepin-music.svg"));
    }
#endif
    if (engine.rootObjects().isEmpty()) {
        qCDebug(dmMusic) << "engine.rootObjects().isEmpty(), return -1";
        return -1;
    }
    // 导入自动播放
    if (!OpenFilePaths.isEmpty()) {
        qCDebug(dmMusic) << "OpenFilePaths: " << OpenFilePaths;
        presenter->importMetas(OpenFilePaths, "play", true);
    }

    QObject::connect(&engine, &QQmlApplicationEngine::quit, presenter.data(), &Presenter::saveDataToDB);

#ifdef Q_OS_LINUX
    // 捕获强制退出信号，保存数据到数据库
    static struct sigaction _sigact;
    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &_sigact, NULL);
#endif

    return app->exec();
}
