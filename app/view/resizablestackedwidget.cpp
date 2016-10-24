/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "resizablestackedwidget.h"

#include <QDebug>

ResizableStackedWidget::ResizableStackedWidget(QWidget *parent) : QStackedWidget(parent)
{
    connect(this, &ResizableStackedWidget::currentChanged,
            this, &ResizableStackedWidget::onCurrentChanged);

}

void ResizableStackedWidget::addWidget(QWidget *pWidget)
{
    pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    QStackedWidget::addWidget(pWidget);
}

void ResizableStackedWidget::onCurrentChanged(int index)
{
    QWidget *pWidget = widget(index);
    Q_ASSERT(pWidget);
    qDebug() << currentIndex() << size();
    pWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pWidget->adjustSize();
    adjustSize();
}
