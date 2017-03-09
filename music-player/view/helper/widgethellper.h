/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QPixmap>
#include <QImage>

namespace WidgetHelper
{

QPixmap blurImage(const QImage &image, int radius);
QImage cropRect(const QImage &image, QSize sz);
QPixmap coverPixmap(const QString &coverPath, QSize sz);

void slideRight2LeftWidget(QWidget *left, QWidget *right, int delay);
void slideBottom2TopWidget(QWidget *top, QWidget *bottom, int delay);
void slideTop2BottomWidget(QWidget *top, QWidget *bottom, int delay);
void slideEdgeWidget(QWidget *right, QRect start, QRect end, int delay, bool hide = false);

void workaround_updateStyle(QWidget *parent, const QString &theme);
}
