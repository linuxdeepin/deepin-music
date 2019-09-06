/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     yub.wang <yub.wang@deepin.io>
 *
 * Maintainer: yub.wang <yub.wang@deepin.io>
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

#include "musicimagebutton.h"

#include <QDebug>
#include <QPainter>

MusicImageButton::MusicImageButton(QWidget *parent)
    : QPushButton(parent)
{
}

MusicImageButton::MusicImageButton(const QString &normalPic, const QString &hoverPic,
                                   const QString &pressPic, QWidget *parent)
    : QPushButton (parent)
{
    normalPicPath = normalPic;
    hoverPicPath = hoverPic;
    pressPicPath = pressPic;
}

void MusicImageButton::paintEvent(QPaintEvent *event)
{
    if (normalPicPath.isEmpty()) {
        QPushButton::paintEvent(event);
        return;
    }
    QString curPicPath = normalPicPath;
    if (status == 1 && !hoverPicPath.isEmpty()) {
        curPicPath = hoverPicPath;
    } else if (status == 2 && !pressPicPath.isEmpty()) {
        curPicPath = hoverPicPath;
    }

    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    QPixmap pixmap(curPicPath);
    painter.drawPixmap(rect(), pixmap, rect());

    painter.restore();
}

void MusicImageButton::enterEvent(QEvent *event)
{
    status = 1;
    QPushButton::enterEvent(event);
}

void MusicImageButton::leaveEvent(QEvent *event)
{
    status = 0;
    QPushButton::leaveEvent(event);
}

void MusicImageButton::mousePressEvent(QMouseEvent *event)
{
    status = 2;
    QPushButton::mousePressEvent(event);
}

void MusicImageButton::mouseReleaseEvent(QMouseEvent *event)
{
    status = 0;
    QPushButton::mouseReleaseEvent(event);
}
