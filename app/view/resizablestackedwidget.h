/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QStackedWidget>

class ResizableStackedWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit ResizableStackedWidget(QWidget *parent = 0);

    void addWidget(QWidget *pWidget);
signals:

public slots:
    void onCurrentChanged(int index);

};

