// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QQuickImageProvider>
#include <QImage>
#include <QIcon>
#include <QString>
#include <QSize>

// Serves freedesktop system-theme icons (looked up via QIcon::fromTheme)
// to QML as "image://theme/<icon-name>". Lets the UI use the system's
// music icon (bloom / bloom-classic-dark / ...) instead of a bundled file.
//
// Usage in QML:
//   Image { source: "image://theme/deepin-music" }
//
// The id may carry a cache-busting fragment ("iconName#themeKey")
// appended by QML to force a reload when the system icon theme changes.
class ThemeIconProvider : public QQuickImageProvider
{
public:
    ThemeIconProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};
