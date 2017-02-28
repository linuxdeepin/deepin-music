/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "lyricview.h"

#include "delegate/lyriclinedelegate.h"

class LyricViewPrivate
{
public:
    LyricViewPrivate(LyricView *parent) : q_ptr(parent) {}

    LyricLineDelegate *delegate = nullptr;

    LyricView *q_ptr;
    Q_DECLARE_PUBLIC(LyricView)
};

LyricView::LyricView(QWidget *parent) :
    QListView(parent), d_ptr(new LyricViewPrivate(this))
{
    Q_D(LyricView);
    d->delegate = new LyricLineDelegate;
    setItemDelegate(d->delegate);
}

LyricView::~LyricView()
{

}
