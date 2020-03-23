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
#include "musicsearchlistview.h"

#include <QDebug>
#include <QVBoxLayout>
#include <DPushButton>
#include <QStringListModel>

#include <DListView>
#include <DApplicationHelper>

#include "pushbutton.h"



SearchResult::SearchResult(QWidget *parent) : DFrame(parent)
{
    QFont labelFont("SourceHanSansSC");
    labelFont.setPointSize(10);
    QPalette labelPalette;
    labelPalette.setColor(QPalette::WindowText, QColor("#414D68 "));

    setFrameRounded(true);

    auto vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(32, 0, 31, 0);
    vlayout->setSpacing(2);
    setLayout(vlayout);

    //音乐
    m_MusicLabel = new DLabel (tr("Music"), this);
    m_MusicLabel->setFont(labelFont);
    m_MusicLabel->setPalette(labelPalette);
    m_MusicLabel->adjustSize();

    m_MusicView = new MusicSearchListview(this);
    m_MusicView->setObjectName("SearchMusicView");
    m_MusicView->setGridSize( QSize(34, 34) );
    m_MusicView->setMinimumWidth(287);
    m_MusicView->adjustSize();

    //演唱者
    m_ArtistLabel = new DLabel (tr("Artists"), this);
    m_ArtistLabel->setFont(labelFont);
    m_ArtistLabel->setPalette(labelPalette);
    m_ArtistLabel->adjustSize();

    m_ArtistView = new MusicSearchListview(this);
    m_ArtistView->setObjectName("SearchArtistView");
    m_ArtistView->setGridSize( QSize(34, 34) );
    m_ArtistView->setMinimumWidth(287);
    m_ArtistView->adjustSize();

    //专辑
    m_AblumLabel = new DLabel (tr("Albums"), this);
    m_AblumLabel->setFont(labelFont);
    m_AblumLabel->setPalette(labelPalette);
    m_AblumLabel->adjustSize();

    m_AlbumView = new MusicSearchListview(this);
    m_AlbumView->setObjectName("SearchAlbumView");
    m_AlbumView->setGridSize( QSize(34, 34) );
    m_AlbumView->setMinimumWidth(287);
    m_AlbumView->adjustSize();

    vlayout->addWidget(m_MusicLabel);
    vlayout->addWidget(m_MusicView);
    vlayout->addWidget(m_ArtistLabel);
    vlayout->addWidget(m_ArtistView);
    vlayout->addWidget(m_AblumLabel);
    vlayout->addWidget(m_AlbumView);

    int themeType = DGuiApplicationHelper::instance()->themeType();
    slotTheme(themeType);
    connect(m_MusicView, &MusicSearchListview::clicked, this, &SearchResult::itemClicked);
    connect(m_ArtistView, &MusicSearchListview::clicked, this, &SearchResult::itemClicked);
    connect(m_AlbumView, &MusicSearchListview::clicked, this, &SearchResult::itemClicked);
}

void SearchResult::autoResize()
{
    int rowCount = m_MusicView->rowCount();

    int musicHeight     = 0;
    int artistHeight    = 0;
    int ablumHeight     = 0;

    m_MusicView->setFixedHeight(rowCount * 34);
    m_MusicView->adjustSize();
    m_MusicView->raise();
    musicHeight = (rowCount == 0 ? 0 : (rowCount + 1) * 34);

    rowCount = m_ArtistView->rowCount();
    m_ArtistView->setFixedHeight(rowCount * 34);
    m_ArtistView->adjustSize();
    m_ArtistView->raise();
    artistHeight = (rowCount == 0 ? 0 : (rowCount + 1) * 34);

    rowCount = m_AlbumView->rowCount();
    m_AlbumView->setFixedHeight(rowCount * 34);
    m_AlbumView->adjustSize();
    m_AlbumView->raise();
    ablumHeight = (rowCount == 0 ? 0 : (rowCount + 1) * 34);


    if (m_MusicView->rowCount() == 0) {
        m_MusicLabel->hide();
        m_MusicView->hide();
    } else {
        m_MusicLabel->show();
        m_MusicView->show();
    }


    if (m_ArtistView->rowCount() == 0) {
        m_ArtistLabel->hide();
        m_ArtistView->hide();
    } else {
        m_ArtistLabel->show();
        m_ArtistView->show();
    }


    if (m_AlbumView->rowCount() == 0) {
        m_AblumLabel->hide();
        m_AlbumView->hide();
    } else {
        m_AblumLabel->show();
        m_AlbumView->show();
    }

    if (m_MusicView->rowCount() == 0
            && m_ArtistView->rowCount() == 0
            && m_AlbumView->rowCount() == 0) {
        hide();
    }

    setFixedHeight(musicHeight
                   + artistHeight
                   + ablumHeight
                   + 10
                  );
}

void SearchResult::setSearchString(const QString &str)
{
    m_MusicView->setProperty("searchString", str);
}

void SearchResult::selectUp()
{
    if (m_Count == 0) {
        return;
    }
    if (m_CurrentIndex < 0) {
        return;
    }
    m_CurrentIndex --;
    if (m_CurrentIndex < m_MusicView->rowCount()) {
        m_MusicView->setCurrentIndexInt(m_CurrentIndex);
        m_ArtistView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);
    } else if (m_CurrentIndex >= m_MusicView->rowCount() - 1
               && m_CurrentIndex < (m_MusicView->rowCount() + m_ArtistView->rowCount())) {
        m_ArtistView->setCurrentIndexInt(m_CurrentIndex
                                         - m_MusicView->rowCount());
        m_MusicView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);
    } else {
        m_AlbumView->setCurrentIndexInt(m_CurrentIndex
                                        - m_MusicView->rowCount()
                                        - m_ArtistView->rowCount());
        m_MusicView->setCurrentIndexInt(-1);
        m_ArtistView->setCurrentIndexInt(-1);
    }
    m_MusicView->repaint();
    m_ArtistView->repaint();
    m_AlbumView->repaint();
    getSearchStr();
}

void SearchResult::selectDown()
{
    if (m_Count == 0) {
        return;
    }
    if (m_CurrentIndex >= m_Count - 1) {
        return;
    }
    m_CurrentIndex ++;
    if (m_CurrentIndex < m_MusicView->rowCount()) {
        m_MusicView->setCurrentIndexInt(m_CurrentIndex);
        m_ArtistView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);
    } else if (m_CurrentIndex >= m_MusicView->rowCount() - 1
               && m_CurrentIndex < (m_MusicView->rowCount() + m_ArtistView->rowCount())) {
        m_ArtistView->setCurrentIndexInt(m_CurrentIndex
                                         - m_MusicView->rowCount());
        m_MusicView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);
    } else {
        m_AlbumView->setCurrentIndexInt(m_CurrentIndex
                                        - m_MusicView->rowCount()
                                        - m_ArtistView->rowCount());
        m_MusicView->setCurrentIndexInt(-1);
        m_ArtistView->setCurrentIndexInt(-1);
    }
    m_MusicView->repaint();
    m_ArtistView->repaint();
    m_AlbumView->repaint();
    getSearchStr();
}

QString SearchResult::currentStr()
{
    QString str;

    //    auto index = m_MusicView->currentIndex();
    //    if (index.isValid()) {
    //        str = m_MusicModel->stringList()[index.row()];
    //    }

    return str;
}

void SearchResult::leaveEvent(QEvent *event)
{
    m_MusicView->setCurrentIndexInt(-1);
    QFrame::leaveEvent(event);
}

void SearchResult::onReturnPressed()
{
    //    auto hashlist = m_MusicModel->property("hashlist").toStringList();
    //    auto selectedRow = m_MusicView->currentIndex().row();
    //    Q_EMIT locateMusic(hashlist.value(selectedRow));
}

void SearchResult::selectPlaylist(PlaylistPtr playlistPtr)
{
    playlist = playlistPtr;
}

void SearchResult::onSearchCand(QString text, PlaylistPtr playlistPtr)
{
    m_CurrentIndex = -1;
    if (playlistPtr->id() == MusicCandListID) {
        m_MusicView->onMusiclistChanged(text, playlistPtr);
    }
    if (playlistPtr->id() == AlbumCandListID) {
        m_AlbumView->onMusiclistChanged(text, playlistPtr);
    }
    if (playlistPtr->id() == ArtistCandListID) {
        m_ArtistView->onMusiclistChanged(text, playlistPtr);
    }
    m_MusicView->setCurrentIndexInt(-1);
    m_AlbumView->setCurrentIndexInt(-1);
    m_ArtistView->setCurrentIndexInt(-1);
    m_Count = m_MusicView->rowCount()
              + m_AlbumView->rowCount()
              + m_ArtistView->rowCount();
    autoResize();
}

void SearchResult::slotTheme(int type)
{
    QPalette labelPalette;
    if (type == 2) {
        labelPalette.setColor(QPalette::WindowText, QColor("#C0C6D4 "));
    } else {
        labelPalette.setColor(QPalette::WindowText, QColor("#414D68 "));
    }
    m_MusicLabel->setPalette(labelPalette);
    m_MusicLabel->setPalette(labelPalette);
    m_MusicLabel->setPalette(labelPalette);
    m_MusicView->setThemeType(type);
    m_AlbumView->setThemeType(type);
    m_ArtistView->setThemeType(type);
}

void SearchResult::itemClicked(QModelIndex index)
{
    index.row();
    PlaylistPtr playList = dynamic_cast<MusicSearchListview *>(index.model()->parent())->playlist();
    QString currentId = playList->id();
    int row = index.row();
    if (currentId == MusicCandListID) {
        Q_EMIT this->searchText2(MusicResultListID, playList->allmusic().at(row)->title);
    }
    if (currentId == AlbumCandListID) {
        Q_EMIT this->searchText2(AlbumResultListID, playList->playMusicTypePtrList().at(row)->name);
    }
    if (currentId == ArtistCandListID) {
        Q_EMIT this->searchText2(ArtistResultListID, playList->playMusicTypePtrList().at(row)->name);
    }

}

void SearchResult::getSearchStr()
{
    if (m_CurrentIndex < 0) {
        return;
    }
    QString text;
    QString id;
    if (m_CurrentIndex < m_MusicView->rowCount()) {
        text = m_MusicView->playlist()->allmusic().at(m_CurrentIndex)->title;
        id = MusicResultListID;
    } else if (m_CurrentIndex >= m_MusicView->rowCount() - 1
               && m_CurrentIndex < (m_MusicView->rowCount() + m_ArtistView->rowCount())) {
        text = m_ArtistView->playlist()->playMusicTypePtrList().at(m_CurrentIndex - m_MusicView->rowCount())->name;
        id = ArtistResultListID;
    } else if (m_CurrentIndex >= m_MusicView->rowCount() + m_ArtistView->rowCount() - 1
               && m_CurrentIndex < (m_MusicView->rowCount() + m_ArtistView->rowCount() + m_AlbumView->rowCount())) {
        text = m_AlbumView->playlist()->playMusicTypePtrList().at(m_CurrentIndex
                                                                  - m_MusicView->rowCount()
                                                                  - m_ArtistView->rowCount() )->name;
        id = AlbumResultListID;
    } else {
        Q_EMIT this->searchText3("", "");
        return;
    }

    Q_EMIT  this->searchText3(id, text);
}
