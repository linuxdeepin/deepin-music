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
#include <QWidget>

class ContentTitlePrivate;
class ContentTitle : public QWidget
{
    Q_OBJECT
public:
    explicit ContentTitle(QWidget *parent = 0);
    ~ContentTitle();

Q_SIGNALS:

public Q_SLOTS:
    void setTitle(const QString &title);

private:
    QScopedPointer<ContentTitlePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ContentTitle)
};

