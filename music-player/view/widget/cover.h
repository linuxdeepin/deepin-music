/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

