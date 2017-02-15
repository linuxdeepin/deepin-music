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
#include <QFrame>

#include <searchmeta.h>

#include "../../core/playlist.h"

class SearchMetaItemPrivate;
class SearchMetaItem : public QFrame
{
    Q_OBJECT
public:
    explicit SearchMetaItem(QWidget *parent = 0);
    ~SearchMetaItem();

    void initUI(const DMusic::SearchMeta& meta);
    void setChecked(bool check);

signals:

public slots:

private:
    QScopedPointer<SearchMetaItemPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), SearchMetaItem)
};

