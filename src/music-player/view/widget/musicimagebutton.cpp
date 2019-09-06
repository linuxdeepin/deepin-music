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
    defaultPicPath.normalPicPath = normalPic;
    defaultPicPath.hoverPicPath = hoverPic;
    defaultPicPath.pressPicPath = pressPic;
}

void MusicImageButton::setPropertyPic(QString propertyName, const QVariant &value,
                                      const QString &normalPic, const QString &hoverPic, const QString &pressPic)
{
    MusicPicPathInfo curPicPath;
    curPicPath.normalPicPath = normalPic;
    curPicPath.hoverPicPath = hoverPic;
    curPicPath.pressPicPath = pressPic;

    if (propertyPicPaths.first == propertyName && propertyPicPaths.second.contains(value)) {
        propertyPicPaths.second[value] = curPicPath;
    } else {
        QMap<QVariant, MusicPicPathInfo> curPicPathInfo;
        curPicPathInfo.insert(value, curPicPath);
        propertyPicPaths.first = propertyName;
        propertyPicPaths.second = curPicPathInfo;
    }
}

void MusicImageButton::paintEvent(QPaintEvent *event)
{
    if (defaultPicPath.normalPicPath.isEmpty()) {
        QPushButton::paintEvent(event);
        return;
    }

    QString curPicPath = defaultPicPath.normalPicPath;
    if (propertyPicPaths.first.isEmpty() || !propertyPicPaths.second.contains(property(propertyPicPaths.first.toStdString().data()))) {
        QString curPropertyPicPathStr;
        if (status == 1 && !defaultPicPath.hoverPicPath.isEmpty()) {
            curPropertyPicPathStr = defaultPicPath.hoverPicPath;
        } else if (status == 2 && !defaultPicPath.pressPicPath.isEmpty()) {
            curPropertyPicPathStr = defaultPicPath.pressPicPath;
        }
        if (!curPropertyPicPathStr.isEmpty()) {
            curPicPath = curPropertyPicPathStr;
        }
    } else {
        QVariant value = property(propertyPicPaths.first.toStdString().data());
        MusicPicPathInfo curPropertyPicPath = propertyPicPaths.second[value];
        QString curPropertyPicPathStr;
        if (status == 1 && !defaultPicPath.hoverPicPath.isEmpty()) {
            curPropertyPicPathStr = curPropertyPicPath.hoverPicPath;
        } else if (status == 2 && !defaultPicPath.pressPicPath.isEmpty()) {
            curPropertyPicPathStr = curPropertyPicPath.pressPicPath;
        } else {
            curPropertyPicPathStr = curPropertyPicPath.normalPicPath;
        }
        if (!curPropertyPicPathStr.isEmpty()) {
            curPicPath = curPropertyPicPathStr;
        }
    }
    QPixmap pixmap(curPicPath);
    if (pixmap.isNull()) {
        pixmap = QPixmap(defaultPicPath.normalPicPath);
    }

    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    painter.drawPixmap(rect(), pixmap);

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
