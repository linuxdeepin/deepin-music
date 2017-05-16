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
#include <thememanager.h>
#include <QGraphicsDropShadowEffect>

#include "pushbutton.h"

SearchResult::SearchResult(QWidget *parent) : QFrame(parent)
{
    ThemeManager::instance()->regisetrWidget(this);

    auto vlayout = new QVBoxLayout();
    setLayout(vlayout);
    vlayout->setContentsMargins(0, 4, 0, 3);
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
    m_doSearchButton->setFixedHeight(25);
    m_doSearchButton->setText(tr("Search \"%1\" in Deepin Music"));
    m_model = new QStringListModel;

    QStringList a;
    a << "1 Music Result" << "2 Music Result";
    a << "3 Music Result" << "4 Music Result";
    a << "5 Music Result" << "6 Music Result";
    a << "7 Music Result" << "8 Music Result";
    a << "9 Music Result" << "10 Music Result";

    m_model->setStringList(a);
    m_searchResult->setModel(m_model);

    vlayout->addWidget(m_searchResult, 0, Qt::AlignHCenter | Qt::AlignTop);
    vlayout->addSpacing(1);
    vlayout->addWidget(m_doSearchButton, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    vlayout->addSpacing(1);

    this->setMinimumHeight(25);
    m_searchResult->setMinimumHeight(25);
    m_searchResult->adjustSize();

    auto *bodyShadow = new QGraphicsDropShadowEffect;
    bodyShadow->setBlurRadius(9.0);
    bodyShadow->setColor(QColor(0, 0, 0, 0.15 * 255));
    bodyShadow->setOffset(0, 4.0);
//    this->setGraphicsEffect(bodyShadow);

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

    connect(m_doSearchButton, &PushButton::pressed,
    this, [ = ]() {
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
    m_searchResult->setFixedHeight(m_model->rowCount() * (25) + 2);
    m_searchResult->setFixedWidth(this->size().width() - 2);
    m_doSearchButton->setFixedWidth(this->size().width() - 2);

    qDebug() << m_doSearchButton->rect() << m_searchResult->rect();
    this->setFixedHeight(m_searchResult->height() + 25 + 8 + 3);
    m_searchResult->setVisible(!(0 == m_model->rowCount()));
    this->adjustSize();
    qDebug() << this->height();
    m_searchResult->raise();
}

void SearchResult::setSearchString(const QString &str)
{
    m_model->setProperty("searchString", str);

    auto searchHits = QString(tr("Search \"%1\" in Deepin Music")).arg(str);
    m_doSearchButton->setText(searchHits);
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
    QFrame::leaveEvent(event);
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
