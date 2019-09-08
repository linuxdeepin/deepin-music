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

ModeButton::ModeButton(QWidget *parent) : DPushButton(parent)
{
    m_mode = 0;
    connect(this, &ModeButton::clicked, this, [ = ]() {
        if (0 == m_modeIcons.length()) {
            return;
        }
        setMode((m_mode + 1) % m_modeIcons.length());

        this->style()->unpolish(this);
        this->style()->polish(this);
        this->update();
    });
}

void ModeButton::setModeIcons(const QStringList &modeIcons)
{
    m_modeIcons = modeIcons;
    m_mode = 0;
    this->style()->unpolish(this);
    this->style()->polish(this);
    this->update();
}

int ModeButton::mode() const
{
    return m_mode;
}

void ModeButton::setMode(int mode)
{
    if (m_mode == mode) {
        return;
    }
    m_mode = mode;

    this->style()->unpolish(this);
    this->style()->polish(this);
    this->update();

    Q_EMIT modeChanged(mode);
}

void ModeButton::paintEvent(QPaintEvent *event)
{
    QString curPicPath = m_modeIcons[m_mode];
    QPixmap pixmap(curPicPath);

    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    painter.drawPixmap(rect(), pixmap, pixmap.rect());

    painter.restore();
}
