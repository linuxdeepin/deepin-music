/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QFrame>

class TitleBar : public QFrame
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = 0);

signals:
    void searchText(const QString& text);
    void locateMusic(const QString& hash);

public slots:
};

#endif // TITLEBAR_H
