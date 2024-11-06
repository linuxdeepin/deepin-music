// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QQmlApplicationEngine>
#include <QScopedPointer>
#include <QQmlContext>
#include <QStandardPaths>
#include <QIcon>
#include <QDBusInterface>
#include <QDBusReply>

#include <DLog>
#include <DApplication>
#include <QSurfaceFormat>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "config.h"

#include "effect/shaderimageview.h"
#include "effect/shaderdataview.h"
#include "presenter.h"
#include "util/eventsfilter.h"
#include "util/shortcut.h"
#include "util/dbusadpator.h"

DWIDGET_USE_NAMESPACE;
DCORE_USE_NAMESPACE;

QScopedPointer<Presenter, QScopedPointerPodDeleter> presenter;

void sig_term_handler(int signum, siginfo_t *info, void *ptr)
{
    qDebug() << "SIGTERM received.";
    presenter->saveDataToDB();
    exit(1);
}

// 此文件是QML应用的启动文件，一般无需修改
int main(int argc, char *argv[])
{
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
    setenv("PULSE_PROP_media.role", "music", 1);

    DmGlobal::checkWaylandMode();

    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setVersion(3, 2);
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

    DApplication *app = new DApplication(argc, argv);
    app->setAttribute(Qt::AA_UseHighDpiPixmaps);
    app->setOrganizationName("deepin");
    app->setApplicationName("deepin-music");
    // Version Time
    app->setApplicationVersion(DApplication::buildVersion(VERSION));

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
        QStringList strList;
        for (QString str : OpenFilePaths) {
            QUrl url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(str));
            strList.append(url.toLocalFile().isEmpty() ? str : url.toLocalFile());
        }
        OpenFilePaths = strList;
    }

    if (!app->setSingleInstance("deepinmusic")) {
        qDebug() << "another deepin music has started";
        QDBusInterface speechbus("org.mpris.MediaPlayer2.DeepinMusic",
                                 "/org/mpris/speech",
                                 "com.deepin.speech",
                                 QDBusConnection::sessionBus());

        if (speechbus.isValid()) {
            QVariant mediaMeta;
            mediaMeta.setValue(OpenFilePaths);
            speechbus.asyncCall("OpenUris", OpenFilePaths);
        }

        QDBusInterface iface("org.mpris.MediaPlayer2.DeepinMusic",
                             "/org/mpris/MediaPlayer2",
                             "org.mpris.MediaPlayer2",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.asyncCall("Raise");
        }
        return 0;
    }

    DmGlobal::initPlaybackEngineType();
    app->setQuitOnLastWindowClosed(false);
    app->loadTranslator();

    // 请在此处注册QML中的C++类型
    qmlRegisterType<ShaderImageView>("audio.image", 1, 0, "View_image");
    qmlRegisterType<ShaderDataView>("audio.image", 1, 0, "View_data");
    qmlRegisterType<DmGlobal>("audio.global", 1, 0, "DmGlobal");

    QString descriptionText = QObject::tr("Music is a local music player with beautiful design and simple functions.");
    QString acknowledgementLink = "https://www.deepin.org/acknowledgments/deepin-music#thanks";
    DmGlobal::setAppName(QObject::tr("Music"));
    qApp->setProductName(DmGlobal::getAppName());
    qApp->setApplicationAcknowledgementPage(acknowledgementLink);
    qApp->setProductIcon(QIcon::fromTheme("deepin-music"));
    qApp->setApplicationDescription(descriptionText);
    qApp->setApplicationDisplayName(DmGlobal::getAppName());

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
    if (engine.rootObjects().isEmpty())
        return -1;
    // 导入自动播放
    if (!OpenFilePaths.isEmpty()) {
        presenter->importMetas(OpenFilePaths, "play", true);
    }

    QObject::connect(&engine, &QQmlApplicationEngine::quit, presenter.data(), &Presenter::saveDataToDB);

    // 捕获强制退出信号，保存数据到数据库
    static struct sigaction _sigact;
    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &_sigact, NULL);

    return app->exec();
}
