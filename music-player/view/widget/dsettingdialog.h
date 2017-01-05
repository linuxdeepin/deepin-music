/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QScopedPointer>
#include "thinwindow.h"
#include <QLabel>

class NavTitle : public QLabel
{
    Q_OBJECT
public:
    explicit NavTitle(QWidget *parent = 0);
};

class NavSubTitle : public QLabel
{
    Q_OBJECT
public:
    explicit NavSubTitle(QWidget *parent = 0);
};

class ContentTitle : public QFrame
{
    Q_OBJECT
public:
    explicit ContentTitle(const QString &title, QWidget *parent = 0);
};

class ContentSubTitle : public QLabel
{
    Q_OBJECT
public:
    explicit ContentSubTitle(QWidget *parent = 0);
};

class DSettingDialogPrivate;
class DSettingDialog : public ThinWindow
{
    Q_OBJECT
public:
    explicit DSettingDialog(QWidget *parent = 0);
    ~DSettingDialog();

signals:
    void mouseMoving(Qt::MouseButton botton);

public slots:

protected:
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<DSettingDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DSettingDialog)
};

