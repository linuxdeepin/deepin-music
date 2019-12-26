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

#include "modebuttom.h"

#include <QStyle>
#include <QPainter>

#include <DHiDPIHelper>
#include <DPalette>

DGUI_USE_NAMESPACE

ModeButton::ModeButton(QWidget *parent) : DPushButton(parent)
{
    m_mode = 0;
    connect(this, &ModeButton::clicked, this, [ = ]() {
        if (0 == m_modeIcons.length()) {
            return;
        }
        setMode((m_mode + 1) % m_modeIcons.length());

        this->update();
    });

    auto pl = this->palette();
    QColor sbcolor("#000000");
    sbcolor.setAlphaF(0.05);
    pl.setColor(DPalette::Shadow, sbcolor);
    setPalette(pl);
}

void ModeButton::setModeIcons(const QStringList &modeIcons, const QStringList &pressIcons)
{
    m_modeIcons = modeIcons;
    m_pressIcons = pressIcons;
    m_mode = 0;
    this->update();
}

int ModeButton::mode() const
{
    return m_mode;
}

void ModeButton::setTransparent(bool flag)
{
    transparent = flag;
}

void ModeButton::setMode(int mode)
{
    if (m_mode == mode) {
        return;
    }
    m_mode = mode;

    this->update();

    Q_EMIT modeChanged(mode);
}

void ModeButton::paintEvent(QPaintEvent *event)
{
    if (!transparent) {
        DPushButton::paintEvent(event);
    }

    QString curPicPath = m_modeIcons[m_mode];
    if (status == 1 && m_mode >= 0 && m_mode <= m_pressIcons.size()) {
        curPicPath = m_pressIcons[m_mode];
    }

//    QPixmap pixmap = DHiDPIHelper::loadNxPixmap(curPicPath);
////    pixmap.setDevicePixelRatio(devicePixelRatioF());

//    QPainter painter(this);
//    painter.save();
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setRenderHint(QPainter::HighQualityAntialiasing);
//    painter.setRenderHint(QPainter::SmoothPixmapTransform);

//    int pixmapWidth = pixmap.rect().width();
//    int pixmapHeight = pixmap.rect().height();
//    QRect pixmapRect((rect().width() - pixmapWidth) / 2, (rect().height() - pixmapHeight) / 2, pixmapWidth, pixmapHeight);
//    pixmapRect = pixmapRect.intersected(rect());
//    painter.drawPixmap(pixmapRect, pixmap, QRect(0, 0, pixmapWidth, pixmapHeight));

//    painter.restore();
//    QIcon icon;
//    icon.addFile(curPicPath);

//    this->setIconSize(QSize(36, 36));
//    this->setIcon(icon);

    DPushButton::paintEvent(event);

    QPixmap pixmap = DHiDPIHelper::loadNxPixmap(curPicPath);

    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    int pixmapWidth = pixmap.rect().width();
    int pixmapHeight = pixmap.rect().height();
    QRect pixmapRect((rect().width() - pixmapWidth) / 2, (rect().height() - pixmapHeight) / 2, pixmapWidth, pixmapHeight);
    pixmapRect = pixmapRect.intersected(rect());
//    painter.drawPixmap(pixmapRect, pixmap, QRect(0, 0, pixmapWidth, pixmapHeight));

    QIcon icon;
    icon.addFile(curPicPath);
    icon.paint(&painter, pixmapRect);

    painter.restore();
}

void ModeButton::mousePressEvent(QMouseEvent *event)
{
    status = 1;
    DPushButton::mousePressEvent(event);
}

void ModeButton::mouseReleaseEvent(QMouseEvent *event)
{
    status = 0;
    DPushButton::mouseReleaseEvent(event);
}
