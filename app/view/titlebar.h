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
#include <dsearchedit.h>

#include "widget/searchresult.h"

DWIDGET_USE_NAMESPACE

class TitleBar : public QFrame
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = 0);

signals:

public slots:
};


class SearchEdit : public DSearchEdit
{
    Q_OBJECT
public:
    explicit SearchEdit(QWidget *parent = 0);

signals:

public slots:
    void onFocusIn();
    void onFocusOut();

private:
    SearchResult *m_result = nullptr;
};

#endif // TITLEBAR_H
