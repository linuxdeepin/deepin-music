/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
