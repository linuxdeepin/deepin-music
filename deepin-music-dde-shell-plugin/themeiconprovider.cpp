// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "themeiconprovider.h"

ThemeIconProvider::ThemeIconProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage ThemeIconProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    // The id may carry a cache-busting fragment ("iconName#themeKey") or
    // query ("iconName?v=1") appended by QML to force a reload when the
    // system icon theme changes. Strip everything after '#' / '?' so
    // QIcon::fromTheme gets the bare icon name.
    QString iconName = id;
    const int hashPos = iconName.indexOf(QLatin1Char('#'));
    if (hashPos >= 0) {
        iconName = iconName.left(hashPos);
    }
    const int queryPos = iconName.indexOf(QLatin1Char('?'));
    if (queryPos >= 0) {
        iconName = iconName.left(queryPos);
    }
    iconName = iconName.trimmed();
    if (iconName.isEmpty()) {
        return {};
    }

    const QIcon icon = QIcon::fromTheme(iconName);
    if (icon.isNull()) {
        return {};
    }

    const QSize targetSize = requestedSize.isValid() && !requestedSize.isEmpty()
        ? requestedSize
        : QSize(64, 64);
    const QPixmap pixmap = icon.pixmap(targetSize);
    if (pixmap.isNull()) {
        return {};
    }

    if (size) {
        *size = pixmap.size();
    }
    return pixmap.toImage();
}
