/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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

#include "musicsearchlistview.h"
#include "musicsearchlistmodel.h"
#include "musicsearchlistdelegate.h"

#include <QTouchEvent>
#include <QDebug>
#include <QFileInfo>
#include <DGuiApplicationHelper>

#include "util/pinyinsearch.h"
#include "util/global.h"
#include "databaseservice.h"
#include "commonservice.h"
#include "searchresult.h"

//DWIDGET_USE_NAMESPACE

MusicSearchListview::MusicSearchListview(QWidget *parent)
    : DListView(parent)
    , m_SearchType(SearchType::none)
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    m_model = new MusicSearchListModel(3, 3, this);
    setModel(m_model);
    m_delegate = new MusicSearchListDelegate(this);
    setItemDelegate(m_delegate);
    setViewportMargins(0, 0, 8, 0);
    setUniformItemSizes(true);

    setViewMode(QListView::ListMode);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setLayoutMode(QListView::Batched);
    setBatchSize(2000);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(m_delegate, &MusicSearchListDelegate::SearchClear, this, &MusicSearchListview::SearchClear);
    connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slotOnClicked(const QModelIndex &)));

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &MusicSearchListview::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());
}


MusicSearchListview::~MusicSearchListview()
{

}

void MusicSearchListview::setSearchResultWidget(SearchResult *result)
{
    m_SearchResultWidget = result;
}

void MusicSearchListview::setSearchType(SearchType type)
{
    m_SearchType = type;
}

SearchType MusicSearchListview::getSearchType() const
{
    return m_SearchType;
}

void MusicSearchListview::setSearchText(QString text)
{
    m_CurrentIndex = -1;
    m_searchText = text;
    int count = 0;
    if (m_SearchType == SearchType::SearchMusic) {
        m_MediaMetas.clear();
        for (MediaMeta &metaData : DataBaseService::getInstance()->allMusicInfos()) {
            if (CommonService::getInstance()->containsStr(text, metaData.title)) {
                m_MediaMetas.append(metaData);
                count ++;
            }
            if (count >= 5) {
                count = 0;
                break;
            }
        }
        m_model->clear();
        for (MediaMeta metaData : m_MediaMetas) {
            QStandardItem *newItem = new QStandardItem;
            m_model->appendRow(newItem);

            auto row = m_model->rowCount() - 1;
            QModelIndex index = m_model->index(row, 0, QModelIndex());

            QVariant mediaMeta;
            mediaMeta.setValue(metaData);
            m_model->setData(index, mediaMeta, Qt::UserRole + SearchType::SearchMusic);
        }
    } else if (m_SearchType == SearchType::SearchSinger) {
        m_SingerInfos.clear();
        for (SingerInfo &singer : DataBaseService::getInstance()->allSingerInfos()) {
            if (CommonService::getInstance()->containsStr(text, singer.singerName)) {
                m_SingerInfos.append(singer);
                count ++;
            }
            if (count >= 3) {
                count = 0;
                break;
            }
        }
        m_model->clear();
        for (SingerInfo singer : m_SingerInfos) {
            QStandardItem *newItem = new QStandardItem;
            m_model->appendRow(newItem);

            auto row = m_model->rowCount() - 1;
            QModelIndex index = m_model->index(row, 0, QModelIndex());

            QVariant singerMeta;
            singerMeta.setValue(singer);
            m_model->setData(index, singerMeta, Qt::UserRole + SearchType::SearchSinger);

            if (singer.musicinfos.values().size() > 0) {
                QVariant IconMeta;
                //设置icon
                bool iconExists = false;
                for (int i = 0; i < singer.musicinfos.values().size(); i++) {
                    MediaMeta metaBind = singer.musicinfos.values().at(i);
                    QString imagesDirPath = Global::cacheDir() + "/images/" + metaBind.hash + ".jpg";
                    QFileInfo file(imagesDirPath);
                    if (file.exists()) {
                        QPixmap icon(imagesDirPath);
                        IconMeta.setValue(icon);
                        m_model->setData(index, IconMeta, Qt::UserRole + SearchType::SearchIcon);
                        iconExists = true;
                        break;
                    }
                }
                if (!iconExists) {
                    IconMeta.setValue(m_defaultIcon);
                    m_model->setData(index, IconMeta, Qt::UserRole + SearchType::SearchIcon);
                }
            }
        }
    } else if (m_SearchType == SearchType::SearchAlbum) {
        m_AlbumInfos.clear();
        for (AlbumInfo &album : DataBaseService::getInstance()->allAlbumInfos()) {
            if (CommonService::getInstance()->containsStr(text, album.albumName)) {
                m_AlbumInfos.append(album);
                count ++;
            }
            if (count >= 3) {
                count = 0;
                break;
            }
        }
        m_model->clear();
        for (AlbumInfo album : m_AlbumInfos) {
            QStandardItem *newItem = new QStandardItem;
            m_model->appendRow(newItem);

            auto row = m_model->rowCount() - 1;
            QModelIndex index = m_model->index(row, 0, QModelIndex());

            QVariant singerMeta;
            singerMeta.setValue(album);
            m_model->setData(index, singerMeta, Qt::UserRole + SearchType::SearchAlbum);

            if (album.musicinfos.values().size() > 0) {
                QVariant IconMeta;
                //设置icon
                bool iconExists = false;
                for (int i = 0; i < album.musicinfos.values().size(); i++) {
                    MediaMeta metaBind = album.musicinfos.values().at(i);
                    QString imagesDirPath = Global::cacheDir() + "/images/" + metaBind.hash + ".jpg";
                    QFileInfo file(imagesDirPath);
                    if (file.exists()) {
                        QPixmap icon(imagesDirPath);
                        IconMeta.setValue(icon);
                        m_model->setData(index, IconMeta, Qt::UserRole + SearchType::SearchIcon);
                        iconExists = true;
                        break;
                    }
                }
                if (!iconExists) {
                    IconMeta.setValue(m_defaultIcon);
                    m_model->setData(index, IconMeta, Qt::UserRole + SearchType::SearchIcon);
                }
            }


        }
    }
}

int MusicSearchListview::rowCount()
{
    return m_model->rowCount();
}

int MusicSearchListview::getIndexInt()const
{
    return m_CurrentIndex;
}

void MusicSearchListview::setThemeType(int type)
{
    m_themeType = type;
}

int MusicSearchListview::getThemeType() const
{
    return m_themeType;
}

QString MusicSearchListview::getSearchText()const
{
    return m_searchText;
}

void MusicSearchListview::setCurrentIndexInt(int row)
{
    m_CurrentIndex = row;
}

void MusicSearchListview::switchToSearchResultTab(const QModelIndex &index)
{
    if (m_SearchType == SearchType::SearchMusic) {
        MediaMeta mediaMeta = index.data(Qt::UserRole + SearchType::SearchMusic).value<MediaMeta>();
        emit CommonService::getInstance()->signalSwitchToView(SearchMusicResultType, mediaMeta.title);
        if (m_SearchResultWidget) {
            m_SearchResultWidget->setLineEditSearchString(mediaMeta.title);
        }
    } else if (m_SearchType == SearchType::SearchSinger) {
        SingerInfo singerInfo = index.data(Qt::UserRole + SearchType::SearchSinger).value<SingerInfo>();
        emit CommonService::getInstance()->signalSwitchToView(SearchSingerResultType, singerInfo.singerName);
        if (m_SearchResultWidget) {
            m_SearchResultWidget->setLineEditSearchString(singerInfo.singerName);
        }
    } else if (m_SearchType == SearchType::SearchAlbum) {
        AlbumInfo albumInfo = index.data(Qt::UserRole + SearchType::SearchAlbum).value<AlbumInfo>();
        emit CommonService::getInstance()->signalSwitchToView(SearchAlbumResultType, albumInfo.albumName);
        if (m_SearchResultWidget) {
            m_SearchResultWidget->setLineEditSearchString(albumInfo.albumName);
        }
    }
}

//QString MusicSearchListview::getCurrentIndexText(int row)
//{
//    QString currentIndexText;
//    if (row >= 0 && row < this->rowCount()) {
//        QModelIndex index = m_model->index(row, 0, QModelIndex());
//        MediaMeta mediaMeta = index.data(Qt::UserRole + SearchType::SearchMusic).value<MediaMeta>();
//        currentIndexText = mediaMeta.title;
//    }
//    return currentIndexText;
//}

//void MusicSearchListview::setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap)
//{
//    m_playingPixmap = pixmap;
//    m_sidebarPixmap = sidebarPixmap;
//    m_albumPixmap = albumPixmap;
//    update();
//}

//QPixmap MusicSearchListview::getPlayPixmap() const
//{
//    return m_playingPixmap;
//}

//QPixmap MusicSearchListview::getSidebarPixmap() const
//{
//    return m_sidebarPixmap;
//}

//QPixmap MusicSearchListview::getAlbumPixmap() const
//{
//    return m_albumPixmap;
//}

//void MusicSearchListview::setViewModeFlag(QListView::ViewMode mode)
//{
//    if (mode == QListView::IconMode) {
//        setIconSize(QSize(170, 170));
//        setGridSize(QSize(170, 170));
//    } else {
//        setIconSize(QSize(36, 36));
//        setGridSize(QSize(-1, -1));
//    }
//    setViewMode(mode);
//}

//void MusicSearchListview::addItem(const QString str)
//{
//    QStandardItem *newItem = new QStandardItem;
//    m_model->appendRow(newItem);

//    auto row = m_model->rowCount() - 1;
//    QModelIndex index = m_model->index(row, 0, QModelIndex());
//    m_model->setData(index, str);
//}

void MusicSearchListview::SearchClear()
{
    Q_EMIT this->sigSearchClear();
}

void MusicSearchListview::slotOnClicked(const QModelIndex &index)
{
    switchToSearchResultTab(index);
}

void MusicSearchListview::onReturnPressed()
{
    qDebug() << "------MusicSearchListview::onReturnPressed";
    slotOnClicked(m_model->index(m_CurrentIndex, 0));
}

void MusicSearchListview::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    qDebug() << "MusicSearchListview::mouseMoveEvent";
}

bool MusicSearchListview::event(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin) {
        QTouchEvent *touch = static_cast<QTouchEvent *>(event);
        if (touch && touch->touchPoints().size() > 0) {
            QModelIndex index = this->indexAt(touch->touchPoints().at(0).pos().toPoint());
            switchToSearchResultTab(index);
        }
    }
    return QListView::event(event);
}
