/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include "label.h"

class CoverPrivate;
class Cover : public Label
{
    Q_OBJECT
    Q_PROPERTY(int radius READ radius WRITE setRadius)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
    Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor NOTIFY shadowColorChanged)

public:
    explicit Cover(QWidget *parent = 0);
    ~Cover();

    int radius() const;
    QColor borderColor() const;
    QColor shadowColor() const;

public slots:
    void setCoverPixmap(const QPixmap &pixmap);
    void setRadius(int radius);
    void setBorderColor(QColor borderColor);
    void setShadowColor(QColor shadowColor);

signals:
    void shadowColorChanged(QColor shadowColor);

protected:
    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    QScopedPointer<CoverPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Cover)
};

