/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MODEBUTTOM_H
#define MODEBUTTOM_H

#include <QPushButton>

class ModeButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    explicit ModeButton(QWidget *parent = 0);

    void setModeIcons(const QStringList &modeIcons);
    int mode() const;
signals:
    void modeChanged(int mode);

public slots:
    void setMode(int mode);

private:
    int         m_mode;
    QStringList m_modeIcons;
};

#endif // MODEBUTTOM_H
