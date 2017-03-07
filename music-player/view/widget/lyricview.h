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
#include <QListView>

class LyricViewPrivate;
class LyricView : public QListView
{
    Q_OBJECT
public:
    explicit LyricView(QWidget *parent = 0);
    ~LyricView();

    bool viewMode() const;
    int optical() const;

    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<LyricViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), LyricView)
};

