/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "modebuttom.h"

#include <QStyle>

ModeButton::ModeButton(QWidget *parent) : QPushButton(parent)
{
    m_mode = 0;
    connect(this, &ModeButton::clicked, this, [ = ]() {
        if (0 == m_modeIcons.length()) {
            return;
        }
        setMode( (m_mode + 1) % m_modeIcons.length());

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

    emit modeChanged(mode);
}
