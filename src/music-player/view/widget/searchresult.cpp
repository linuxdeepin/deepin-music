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

#include "searchresult.h"

#include <QDebug>
#include <QVBoxLayout>
#include <DPushButton>
#include <QStringListModel>

#include <DListView>
#include <DApplicationHelper>

#include "pushbutton.h"

SearchResult::SearchResult(QWidget *parent) : DFrame(parent)
{
    auto vlayout = new QVBoxLayout();
    setLayout(vlayout);
    vlayout->setContentsMargins(0, 4, 0, 4);
    vlayout->setSpacing(0);

    //setFixedWidth(350);
    setFrameRounded(true);

    m_searchResult = new DListView(this);
    m_searchResult->setMouseTracking(true);
    m_searchResult->setObjectName("SearchResultList");
    m_searchResult->setSelectionMode(QListView::SingleSelection);
    m_searchResult->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_searchResult->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_searchResult->setFocusPolicy(Qt::NoFocus);
    m_searchResult->setIconSize( QSize(34, 34) );
    m_searchResult->setGridSize( QSize(34, 34) );
    m_searchResult->setItemSize(QSize(34, 34));
    m_searchResult->setItemSpacing(1);
    m_searchResult->setViewportMargins(0, 0, 0, 0);
    //m_searchResult->setFixedWidth(348);

    m_searchResult->setFrameShape(QFrame::NoFrame);
    DPalette pa = DApplicationHelper::instance()->palette(m_searchResult);
    pa.setColor(DPalette::ItemBackground, Qt::transparent);
    DApplicationHelper::instance()->setPalette(m_searchResult, pa);

    m_model = new QStringListModel;

    QStringList a;
    m_model->setStringList(a);
    m_searchResult->setModel(m_model);

    vlayout->addWidget(m_searchResult, 100, Qt::AlignHCenter | Qt::AlignTop);

    m_searchResult->adjustSize();

    connect(m_searchResult, &QListView::clicked,
    this, [ = ](const QModelIndex & index) {
        auto hashlist = m_model->property("hashlist").toStringList();
        Q_EMIT locateMusic(hashlist.value(index.row()));
        Q_EMIT searchText(m_model->stringList()[index.row()]);
    });
}

void SearchResult::autoResize()
{
    int rowCount = m_model->rowCount();
    if (rowCount > 10)
        rowCount = 10;
    m_searchResult->setFixedHeight(rowCount * 34);
    m_searchResult->setFixedWidth(width() - 8);

    setFixedHeight(m_searchResult->height() + 8);

    m_searchResult->setVisible(!(0 == m_model->rowCount()));
    this->adjustSize();
    qDebug() << this->height();
    m_searchResult->raise();
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

void SearchResult::selectUp()
{
    if (!m_searchResult->currentIndex().isValid()) {
        auto newSelectedRow = m_model->rowCount() - 1;
        QModelIndex indexOfTheCellIWant = m_model->index(newSelectedRow, 0);
        m_searchResult->setCurrentIndex(indexOfTheCellIWant);
    } else {
        auto newSelectedRow = m_searchResult->currentIndex().row() - 1;
        if (newSelectedRow < 0) {
            m_searchResult->setCurrentIndex(QModelIndex());
            return;
        }
        QModelIndex indexOfTheCellIWant = m_model->index(newSelectedRow, 0);
        m_searchResult->setCurrentIndex(indexOfTheCellIWant);
    }
}

void SearchResult::selectDown()
{
    auto newSelectedRow = m_searchResult->currentIndex().row() + 1;
    if (newSelectedRow >= m_model->rowCount()) {
        m_searchResult->setCurrentIndex(QModelIndex());
        return;
    }
    QModelIndex indexOfTheCellIWant = m_model->index(newSelectedRow, 0);
    m_searchResult->setCurrentIndex(indexOfTheCellIWant);
}

QString SearchResult::currentStr()
{
    QString str;

    auto index = m_searchResult->currentIndex();
    if (index.isValid()) {
        str = m_model->stringList()[index.row()];
    }

    return str;
}

void SearchResult::leaveEvent(QEvent *event)
{
    m_searchResult->setCurrentIndex(QModelIndex());
    QFrame::leaveEvent(event);
}

void SearchResult::onReturnPressed()
{
    auto hashlist = m_model->property("hashlist").toStringList();
    auto selectedRow = m_searchResult->currentIndex().row();
    Q_EMIT locateMusic(hashlist.value(selectedRow));
}

void SearchResult::selectPlaylist(PlaylistPtr playlistPtr)
{
    playlist = playlistPtr;
}
