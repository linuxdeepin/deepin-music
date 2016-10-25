/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef SLIDER_H
#define SLIDER_H

#include <QSlider>

class Slider : public QSlider
{
    Q_OBJECT
public:
    explicit Slider(QWidget *parent = 0);
    explicit Slider(Qt::Orientation orientation, QWidget *parent = Q_NULLPTR);

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
signals:

public slots:
};

#endif // SLIDER_H
