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
#include "pushbutton.h"

SearchResult::SearchResult(QWidget *parent) : DAbstractDialog(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint
                   | Qt::WindowStaysOnTopHint | Qt::WindowSystemMenuHint);

    auto vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(4, 4, 4, 4);
    vlayout->setSpacing(0);

    m_searchResult = new QListView;
    m_searchResult->setMouseTracking(true);
    m_searchResult->setObjectName("SearchResultList");
    m_searchResult->setSelectionMode(QListView::SingleSelection);
    m_searchResult->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_searchResult->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_searchResult->setFocusPolicy(Qt::NoFocus);

    m_doSearchButton = new PushButton;
    m_doSearchButton->setCheckable(true);
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

    this->setMinimumHeight(25);
    m_searchResult->setMinimumHeight(25);
    m_searchResult->adjustSize();
    this->adjustSize();

    D_THEME_INIT_WIDGET(Widget/SearchResult);

    connect(m_searchResult, &QListView::clicked,
    this, [ = ](const QModelIndex & index) {
        auto hashlist = m_model->property("hashlist").toStringList();
        emit locateMusic(hashlist.value(index.row()));
    });
    connect(m_searchResult, &QListView::entered,
    this, [ = ](const QModelIndex & index) {
        m_searchResult->setCurrentIndex(index);
        m_doSearchButton->setChecked(false);
    });

    connect(m_doSearchButton, &PushButton::clicked,
    this, [ = ](bool) {
        auto str = m_model->property("searchString").toString();
        emit searchText(str);
    });
    connect(m_doSearchButton, &PushButton::entered,
    this, [ = ]() {
        m_searchResult->setCurrentIndex(QModelIndex());
        m_doSearchButton->setChecked(true);
    });
}

void SearchResult::autoResize()
{
    m_searchResult->setFixedHeight(m_model->rowCount() * 25);
    this->adjustSize();
}

void SearchResult::setSearchString(const QString &str)
{
    m_model->setProperty("searchString", str);
}

void SearchResult::setResultList(const QStringList &titlelist, const QStringList &hashlist)
{
    m_model->setStringList(titlelist);
    m_model->setProperty("hashlist", hashlist);
}

bool SearchResult::isSelected()
{
    return m_doSearchButton->isChecked() || (m_searchResult->currentIndex().row() >= 0);
}

void SearchResult::selectUp()
{
    if (0 == m_model->rowCount()) {
        m_doSearchButton->setChecked(true);
    }
    if (m_doSearchButton->isChecked()) {
        m_doSearchButton->setChecked(false);
        QModelIndex indexOfTheCellIWant = m_model->index(m_model->rowCount() - 1, 0);
        m_searchResult->setCurrentIndex(indexOfTheCellIWant);
    } else {
        auto newSelectedRow = m_searchResult->currentIndex().row() - 1;
        if (newSelectedRow < 0) {
            m_searchResult->setCurrentIndex(QModelIndex());
            m_doSearchButton->setChecked(true);
            return;
        }
        QModelIndex indexOfTheCellIWant = m_model->index(newSelectedRow, 0);
        m_searchResult->setCurrentIndex(indexOfTheCellIWant);
    }
}

void SearchResult::selectDown()
{
    if (0 == m_model->rowCount()) {
        m_doSearchButton->setChecked(true);
    }

    if (m_doSearchButton->isChecked()) {
        m_doSearchButton->setChecked(false);
        QModelIndex indexOfTheCellIWant = m_model->index(0, 0);
        m_searchResult->setCurrentIndex(indexOfTheCellIWant);
    } else {
        auto newSelectedRow = m_searchResult->currentIndex().row() + 1;
        if (newSelectedRow >= m_model->rowCount()) {
            m_searchResult->setCurrentIndex(QModelIndex());
            m_doSearchButton->setChecked(true);
            return;
        }
        QModelIndex indexOfTheCellIWant = m_model->index(newSelectedRow, 0);
        m_searchResult->setCurrentIndex(indexOfTheCellIWant);
    }
}

void SearchResult::leaveEvent(QEvent *event)
{
    m_searchResult->setCurrentIndex(QModelIndex());
    m_doSearchButton->setChecked(false);
    DAbstractDialog::leaveEvent(event);
}

void SearchResult::onReturnPressed()
{
    if (m_doSearchButton->isChecked()) {
        emit this->searchText(m_model->property("searchString").toString());
    } else {
        auto hashlist = m_model->property("hashlist").toStringList();
        auto selectedRow = m_searchResult->currentIndex().row();
        emit locateMusic(hashlist.value(selectedRow));
    }
}
