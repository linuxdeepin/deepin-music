/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QFrame>

class Tip : public QFrame
{
    Q_OBJECT
public:
    explicit Tip(const QPixmap &icon,
                 const QString &text,
                 QWidget *parent = 0);

    void pop(QPoint center);
    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

   QFrame *m_interFrame = nullptr;
};
