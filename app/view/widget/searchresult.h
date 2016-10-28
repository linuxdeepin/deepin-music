/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef SEARCHRESULTLIST_H
#define SEARCHRESULTLIST_H

#include <dabstractdialog.h>
DWIDGET_USE_NAMESPACE

class FitSizeLstView;
class QPushButton;
class QStringListModel;

class SearchResult : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit SearchResult(QWidget *parent = 0);

    virtual void focusInEvent(QFocusEvent *event);
signals:

public slots:
private:
    FitSizeLstView      *m_searchResult = nullptr;
    QPushButton         *m_doSearchButton = nullptr;
    QStringListModel    *m_model = nullptr;
};

#endif // SEARCHRESULTLIST_H
