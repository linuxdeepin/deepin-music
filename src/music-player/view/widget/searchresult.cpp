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

#include "pushbutton.h"

SearchResult::SearchResult(QWidget *parent) : DFloatingWidget(parent)
{
    auto vlayout = new QVBoxLayout();
    setLayout(vlayout);
    vlayout->setContentsMargins(2, 0, 2, 0);
    vlayout->setSpacing(0);

    setFixedWidth(354);

    m_searchResult = new DListView;
    m_searchResult->setMouseTracking(true);
    m_searchResult->setObjectName("SearchResultList");
    m_searchResult->setSelectionMode(QListView::SingleSelection);
    m_searchResult->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_searchResult->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_searchResult->setFocusPolicy(Qt::NoFocus);
    m_searchResult->setIconSize( QSize(34, 34) );
    m_searchResult->setGridSize( QSize(34, 34) );
    m_searchResult->setViewportMargins(0, 0, 0, 0);
    m_searchResult->setFixedWidth(350);

    m_model = new QStringListModel;

    QStringList a;
    m_model->setStringList(a);
    m_searchResult->setModel(m_model);

    vlayout->addWidget(m_searchResult, 0, Qt::AlignHCenter | Qt::AlignTop);

    m_searchResult->adjustSize();

    connect(m_searchResult, &QListView::clicked,
    this, [ = ](const QModelIndex & index) {
        auto hashlist = m_model->property("hashlist").toStringList();
        Q_EMIT locateMusic(hashlist.value(index.row()));
    });
}

void SearchResult::autoResize()
{
    m_searchResult->setFixedHeight(m_model->rowCount() * 34 + 2);

    setFixedHeight(m_searchResult->height() + 11);

    m_searchResult->setVisible(!(0 == m_model->rowCount()));
    this->adjustSize();
    qDebug() << this->height();
    m_searchResult->raise();
}

void SearchResult::setSearchString(const QString &str)
{
    m_model->setProperty("searchString", str);
    //playlist
    QStringList curList;
    if (playlist->id() == AlbumMusicListID || playlist->id() == ArtistMusicListID) {
        PlayMusicTypePtrList playMusicTypePtrList = playlist->playMusicTypePtrList();

        for (auto action : playMusicTypePtrList) {
            curList.append(action->name);
        }
    } else {
        for (auto action : playlist->allmusic()) {
            curList.append(action->title);
        }
    }
    curList = curList.filter(str);
    m_model->setStringList(curList);
}

void SearchResult::setResultList(const QStringList &titlelist, const QStringList &hashlist)
{
    m_model->setStringList(titlelist);
    m_model->setProperty("hashlist", hashlist);
}

void SearchResult::selectUp()
{
    auto newSelectedRow = m_searchResult->currentIndex().row() - 1;
    if (newSelectedRow < 0) {
        m_searchResult->setCurrentIndex(QModelIndex());
        return;
    }
    QModelIndex indexOfTheCellIWant = m_model->index(newSelectedRow, 0);
    m_searchResult->setCurrentIndex(indexOfTheCellIWant);
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

void SearchResult::leaveEvent(QEvent *event)
{
    m_searchResult->setCurrentIndex(QModelIndex());
    DFloatingWidget::leaveEvent(event);
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
