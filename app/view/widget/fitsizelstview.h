/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef FITSIZELSTVIEW_H
#define FITSIZELSTVIEW_H

#include <QListView>

class FitSizeLstView : public QListView
{
    Q_OBJECT
public:
    explicit FitSizeLstView(QWidget *parent = 0);

    QSize sizeHint() const Q_DECL_OVERRIDE;
signals:

public slots:
};

#endif // FITSIZELSTVIEW_H
