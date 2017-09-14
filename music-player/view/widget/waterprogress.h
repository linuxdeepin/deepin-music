/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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

#include <QScopedPointer>
#include <QWidget>

class WaterProgressInter;
class WaterProgressBorder;
class WaterProgress : public QWidget
{
    Q_OBJECT
public:
    explicit WaterProgress(QWidget *parent = 0);

    void updateSize(QSize size);

private:
    WaterProgressInter *water;
    WaterProgressBorder *waterBoder;
};

class WaterProgressInterPrivate;
class WaterProgressInter : public QWidget
{
    Q_OBJECT
public:
    explicit WaterProgressInter(QWidget *parent = 0);
    ~WaterProgressInter();

public slots:
    void start();
    void stop();
    void setProgress(int);

protected:
    virtual void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

private:
    QScopedPointer<WaterProgressInterPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), WaterProgressInter)
};

class WaterProgressBorder : public QWidget
{
    Q_OBJECT
public:
    explicit WaterProgressBorder(QWidget *parent = 0);
    virtual void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
};

