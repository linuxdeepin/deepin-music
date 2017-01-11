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
#include <dtitlebar.h>

class TitlebarPrivate;
class Titlebar : public Dtk::Widget::DTitlebar
{
    Q_OBJECT

    Q_PROPERTY(QBrush background READ background WRITE setBackground)
    Q_PROPERTY(QColor borderBottom READ borderBottom WRITE setBorderBottom)
    Q_PROPERTY(QColor borderShadowTop READ borderShadowTop WRITE setBorderShadowTop)
    Q_PROPERTY(QString viewname READ viewname WRITE setViewname NOTIFY viewnameChanged)

public:
    explicit Titlebar(QWidget *parent = 0);
    ~Titlebar();

    QString viewname() const;
    QBrush background() const;
    QColor borderBottom() const;
    QColor borderShadowTop() const;

signals:
    void viewnameChanged(QString viewname);

public slots:
    void setViewname(QString viewname);
    void setBackground(QBrush background);
    void setBorderBottom(QColor borderBottom);
    void setBorderShadowTop(QColor borderShadowTop);

protected:
    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    QScopedPointer<TitlebarPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Titlebar)
    QColor m_borderBottom;
};
