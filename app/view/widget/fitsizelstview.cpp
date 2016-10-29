/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "fitsizelstview.h"

#include <QDebug>
FitSizeLstView::FitSizeLstView(QWidget *parent) : QListView(parent)
{

}

QSize FitSizeLstView::sizeHint() const
{
    if (model()->rowCount() == 0) { return QSize(width(), 0); }
//    int nToShow = _nItemsToShow < model()->rowCount() ? _nItemsToShow : model()->rowCount();
    int nToShow = model()->rowCount();
    qDebug() << nToShow;
    qDebug () << nToShow * sizeHintForRow(0) << minimumSize();
    return QSize(width(), nToShow * sizeHintForRow(0));
}
