// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QDebug>

#include "musicapplet.h"
#include "themeiconprovider.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("deepin-music-dde-shell-plugin"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("theme"), new ThemeIconProvider);

    auto *applet = new MusicApplet(&app);

    // qmlRegisterSingletonInstance requires the object to NOT be parented
    // to the engine; QGuiApplication will clean it up at shutdown.
    qmlRegisterSingletonInstance("org.deepin.music", 1, 0,
                                "MusicApplet",
                                applet);

    const QUrl url(QStringLiteral("qrc:/org/deepin/music/package/main.qml"));
    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        qWarning() << "No root objects loaded!";
        return -1;
    }

    return app.exec();
}
