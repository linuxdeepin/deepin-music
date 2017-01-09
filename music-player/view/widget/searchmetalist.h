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
#include <QListWidget>

class SearchMetaListPrivate;
class SearchMetaList : public QListWidget
{
    Q_OBJECT
public:
    explicit SearchMetaList(QWidget *parent = 0);
    ~SearchMetaList();

signals:

public slots:

private:
    QScopedPointer<SearchMetaListPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), SearchMetaList)
};

