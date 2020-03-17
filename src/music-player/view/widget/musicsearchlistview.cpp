/*
 * Copyright (C) 2020 chengdu Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <zhaoyongming@uniontech.com>
 *
 * Maintainer: Iceyer <zhaoyongming@uniontech.com>
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
#include "model/musicsearchlistmodel.h"
#include "delegate/musicsearchlistdelegate.h"

#include <QDebug>
#include "util/pinyinsearch.h"

DWIDGET_USE_NAMESPACE

class MusicSearchListviewPrivate
{
public:
    MusicSearchListviewPrivate(MusicSearchListview *parent): q_ptr(parent) {}
    void addItem(const QString str);

    MusicSearchListModel        *model        = nullptr;
    MusicSearchListDelegate     *delegate     = nullptr;
    PlaylistPtr                 playList      = nullptr;
    QString                     searchText     = "";
    PlayMusicTypePtrList        searchMusicTypePtrList;
    MetaPtrList                 searchMetaPtrPtrList;
    int                         themeType     = 1;
    int                         m_CurrentIndex = 0;
    QString                     defaultCover = ":/images/logo.svg";
    QPixmap                     playingPixmap = QPixmap(":/images/logo.svg");
    QPixmap                     sidebarPixmap = QPixmap(":/images/logo.svg");
    QPixmap                     albumPixmap   = QPixmap(":/images/logo.svg");
    MusicSearchListview         *q_ptr;
    Q_DECLARE_PUBLIC(MusicSearchListview)
};

void MusicSearchListviewPrivate::addItem(const QString str)
{
    QStandardItem *newItem = new QStandardItem;
    model->appendRow(newItem);

    auto row = model->rowCount() - 1;
    QModelIndex index = model->index(row, 0, QModelIndex());
    model->setData(index, str);
}


MusicSearchListview::MusicSearchListview(QWidget *parent)
    : DListView(parent), d_ptr(new MusicSearchListviewPrivate(this))
{
    Q_D(MusicSearchListview);

    d->model = new MusicSearchListModel (3, 3, this);
    setModel(d->model);
    d->delegate = new MusicSearchListDelegate;
    setItemDelegate(d->delegate);
    setViewportMargins(0, 0, 8, 0);

    setUniformItemSizes(true);

    setViewModeFlag(QListView::ListMode);
    setResizeMode( QListView::Adjust );
    setMovement( QListView::Static );
    setLayoutMode(QListView::Batched);
    setBatchSize(2000);

    //setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}


MusicSearchListview::~MusicSearchListview()
{

}

PlaylistPtr MusicSearchListview::playlist() const
{
    Q_D(const MusicSearchListview);
    return d->model->playlist();
}

int MusicSearchListview::rowCount()
{
    Q_D(const MusicSearchListview);
    return d->model->rowCount();
}

int MusicSearchListview::listSize()
{
    Q_D(MusicSearchListview);
    return d->searchMusicTypePtrList.size();
}

int MusicSearchListview::getIndexInt()const
{
    Q_D(const MusicSearchListview);
    return d->m_CurrentIndex;
}

void MusicSearchListview::onMusiclistChanged(QString text, PlaylistPtr playlist)
{
    Q_D(MusicSearchListview);

    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }
    d->searchText = text;
    setUpdatesEnabled(false);
    d->model->removeRows(0, d->model->rowCount());

    QString searchStr = playlist->searchStr();
    bool chineseFlag = false;
    for (auto ch : searchStr) {
        if (DMusic::PinyinSearch::isChinese(ch)) {
            chineseFlag = true;
            break;
        }
    }
    if (playlist->id() == MusicCandListID) {
        d->searchMetaPtrPtrList.clear();
        for (auto meta : playlist->allmusic()) {
            d->addItem(meta->title + "-" + meta->artist);
            d->searchMetaPtrPtrList.append(meta);
        }
    } else {
        d->searchMusicTypePtrList.clear();
        for (auto meta : playlist->playMusicTypePtrList()) {
            if (searchStr.isEmpty()) {
                d->addItem(meta->name);
                d->searchMusicTypePtrList.append(meta);
            } else {
                if (chineseFlag) {
                    if (meta->name.contains(searchStr, Qt::CaseInsensitive)) {
                        d->addItem(meta->name);
                        d->searchMusicTypePtrList.append(meta);
                    }
                } else {
                    if (playlist->searchStr().size() == 1) {
                        auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(meta->name);
                        if (!curTextList.isEmpty() && curTextList.first().contains(searchStr, Qt::CaseInsensitive)) {
                            d->addItem(meta->name);
                            d->searchMusicTypePtrList.append(meta);
                        }
                    } else {
                        auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(meta->name);
                        if (!curTextList.isEmpty() && curTextList.join("").contains(searchStr, Qt::CaseInsensitive)) {
                            d->addItem(meta->name);
                            d->searchMusicTypePtrList.append(meta);
                        }
                    }
                }
            }
        }

    }
    d->model->setPlaylist(playlist);
    setUpdatesEnabled(true);
}

MetaPtrList MusicSearchListview::playMetaList() const
{
    Q_D(const MusicSearchListview);
    return d->searchMetaPtrPtrList;
}

PlayMusicTypePtrList MusicSearchListview::playMusicTypePtrList() const
{
    Q_D(const MusicSearchListview);
    return d->searchMusicTypePtrList;
}

void MusicSearchListview::setThemeType(int type)
{
    Q_D(MusicSearchListview);
    d->themeType = type;
}

int MusicSearchListview::getThemeType() const
{
    Q_D(const MusicSearchListview);
    return d->themeType;
}

QString MusicSearchListview::getSearchText()const
{
    Q_D(const MusicSearchListview);
    return d->searchText;
}

void MusicSearchListview::setCurrentIndexInt(int row)
{
    Q_D(MusicSearchListview);
    d->m_CurrentIndex = row;
}

void MusicSearchListview::setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap)
{
    Q_D(MusicSearchListview);
    d->playingPixmap = pixmap;
    d->sidebarPixmap = sidebarPixmap;
    d->albumPixmap = albumPixmap;
    update();
}

QPixmap MusicSearchListview::getPlayPixmap() const
{
    Q_D(const MusicSearchListview);
    return d->playingPixmap;
}

QPixmap MusicSearchListview::getSidebarPixmap() const
{
    Q_D(const MusicSearchListview);
    return d->sidebarPixmap;
}

QPixmap MusicSearchListview::getAlbumPixmap() const
{
    Q_D(const MusicSearchListview);
    return d->albumPixmap;
}

void MusicSearchListview::updateList()
{
    //更新model数据
    Q_D(MusicSearchListview);
    PlaylistPtr playlist = d->model->playlist();
    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }

    QVector<QString> allStr;
    for (auto meta : playlist->playMusicTypePtrList()) {
        allStr.append(meta->name);
    }

    for (int i = d->model->rowCount() - 1; i >= 0; --i) {
        auto index = d->model->index(i, 0);
        auto itemName = d->model->data(index).toString();
        if (!allStr.contains(itemName)) {
            d->searchMusicTypePtrList.removeAt(i);
            d->model->removeRow(i);
        }
    }
}

void MusicSearchListview::keyChoice()
{
    Q_D(MusicSearchListview);
    qDebug() << "cle down" << endl;

    QModelIndex indexFromList = d->model->index(1, 0);//取model
    this->setCurrentIndex(indexFromList);
}


void MusicSearchListview::setViewModeFlag(QListView::ViewMode mode)
{
    if (mode == QListView::IconMode) {
        setIconSize( QSize(170, 170) );
        setGridSize( QSize(170, 170) );
    } else {
        setIconSize( QSize(36, 36) );
        setGridSize( QSize(-1, -1) );
    }
    setViewMode(mode);
}

void MusicSearchListview::mouseMoveEvent(QMouseEvent *event)
{
    //    qDebug() << "MusicSearchListview::mouseMoveEvent";
}
