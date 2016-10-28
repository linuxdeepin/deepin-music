/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "searchresult.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QListView>
#include <QPushButton>
#include <QStringListModel>
#include <dlistview.h>
#include <dthememanager.h>

#include "fitsizelstview.h"

SearchResult::SearchResult(QWidget *parent) : DAbstractDialog(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint
                   | Qt::WindowStaysOnTopHint | Qt::WindowSystemMenuHint);

    auto vlayout = new QVBoxLayout(this);
    vlayout->setMargin(5);
    vlayout->setSpacing(0);

    m_searchResult = new FitSizeLstView;
    m_searchResult->setObjectName("SearchResultList");
    m_searchResult->setSelectionMode(QListView::SingleSelection);
    m_searchResult->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_searchResult->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_searchResult->setFocusPolicy(Qt::NoFocus);

    m_doSearchButton = new QPushButton;
    m_doSearchButton->setObjectName("SearchResultAction");
    m_doSearchButton->setFixedHeight(24);
    m_doSearchButton->setText(tr("Search for ..."));
    m_model = new QStringListModel;

    QStringList a;
    a << "1 Music Result" << "2 Music Result";
    a << "3 Music Result" << "4 Music Result";
    a << "5 Music Result" << "6 Music Result";
    a << "7 Music Result" << "8 Music Result";
    a << "9 Music Result" << "10 Music Result";

    m_model->setStringList(a);
    m_searchResult->setModel(m_model);

    vlayout->addWidget(m_searchResult, 0, Qt::AlignCenter);
    vlayout->addWidget(m_doSearchButton);

    m_searchResult->adjustSize();
    this->adjustSize();

    D_THEME_INIT_WIDGET(Widget/SearchResult);
}

void SearchResult::focusInEvent(QFocusEvent *event)
{
    qDebug() << event;
}
