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

#include "musiclistdataview.h"

#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QResizeEvent>
#include <QStandardItemModel>

#include <DMenu>
#include <DDialog>
#include <DDesktopServices>
#include <DScrollBar>

#include "util/pinyinsearch.h"

#include "../../core/metasearchservice.h"
#include "../helper/widgethellper.h"

#include "delegate/musiclistdatadelegate.h"
#include "model/musiclistdatamodel.h"
#include "musiclistdialog.h"

DWIDGET_USE_NAMESPACE

class MusicListDataViewPrivate
{
public:
    MusicListDataViewPrivate(MusicListDataView *parent): q_ptr(parent) {}

    void addPlayMusicTypePtr(const PlayMusicTypePtr TypePtr);

    MusicListDataModel      *model        = nullptr;
    MusicListDataDelegate   *delegate     = nullptr;
    QString                 defaultCover = ":/common/image/cover_max.svg";
    MetaPtr                 playing       = nullptr;

    int                     themeType     = 1;

    MusicListDialog        *musciListDialog = nullptr;
    PlayMusicTypePtrList    curPlayMusicTypePtrList;
    QPixmap                 playingPixmap = QPixmap(":/mpimage/light/music1.svg");

    MusicListDataView *q_ptr;
    Q_DECLARE_PUBLIC(MusicListDataView)
};


void MusicListDataViewPrivate::addPlayMusicTypePtr(const PlayMusicTypePtr TypePtr)
{
    QStandardItem *newItem = new QStandardItem;
    QIcon icon(defaultCover);
    if (!TypePtr->icon.isEmpty()) {
        icon = QIcon(QPixmap::fromImage(QImage::fromData(TypePtr->icon).scaled(170, 170)));
    }
    newItem->setIcon(icon);
    model->appendRow(newItem);

    auto row = model->rowCount() - 1;
    QModelIndex index = model->index(row, 0, QModelIndex());
    model->setData(index, TypePtr->name);
}


MusicListDataView::MusicListDataView(QWidget *parent)
    : ListView(parent), d_ptr(new MusicListDataViewPrivate(this))
{
    Q_D(MusicListDataView);

    setObjectName("MusicListDataView");

    d->model = new MusicListDataModel(0, 1, this);
    setModel(d->model);

    d->delegate = new MusicListDataDelegate;
    setItemDelegate(d->delegate);

    setViewModeFlag(QListView::ListMode);
    setResizeMode( QListView::Adjust );
    setMovement( QListView::Static );
    setLayoutMode(QListView::Batched);
    setBatchSize(20);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    d->musciListDialog = new MusicListDialog(this);


    connect(this, &MusicListDataView::doubleClicked,
    this, [ = ](const QModelIndex & index) {
        PlaylistPtr curPlaylist = d->model->playlist();
        auto playMusicTypePtrList = curPlaylist->playMusicTypePtrList();
        if (index.row() >= playMusicTypePtrList.size()) {
            return;
        }
        auto PlayMusicTypePtr = playMusicTypePtrList[index.row()];

        d->musciListDialog->setPlayMusicData(curPlaylist, PlayMusicTypePtr);
        d->musciListDialog->setPlaying(playing());
        d->musciListDialog->exec();
    });

    connect(d->musciListDialog, &MusicListDialog::requestCustomContextMenu,
    this, [ = ](const QPoint & pos) {
        Q_EMIT requestCustomContextMenu(pos);
    });

    connect(d->musciListDialog, &MusicListDialog::playMedia,
    this, [ = ](const MetaPtr meta) {
        Q_EMIT playMedia(meta);
    });

    connect(d->musciListDialog, &MusicListDialog::resume,
    this, [ = ](const MetaPtr meta) {
        Q_EMIT resume(meta);
    });

    connect(d->musciListDialog, &MusicListDialog::addToPlaylist,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList  & metalist) {
        Q_EMIT addToPlaylist(playlist, metalist);
    });

    connect(d->musciListDialog, &MusicListDialog::musiclistRemove,
    this, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT musiclistRemove(metalist);
    });

    connect(d->musciListDialog, &MusicListDialog::musiclistDelete,
    this, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT musiclistDelete(metalist);
    });

    connect(d->musciListDialog, &MusicListDialog::modeChanged,
    this, [ = ](int mode) {
        Q_EMIT modeChanged(mode);
    });

    connect(d->musciListDialog, &MusicListDialog::pause,
    this, [ = ](const MetaPtr meta) {
        Q_EMIT pause(playlist(), meta);
    });

    setSelectionMode(QListView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

MusicListDataView::~MusicListDataView()
{

}

PlaylistPtr MusicListDataView::playlist() const
{
    Q_D(const MusicListDataView);
    return d->model->playlist();
}

int MusicListDataView::listSize()
{
    Q_D(MusicListDataView);
    return d->curPlayMusicTypePtrList.size();
}

void MusicListDataView::setViewModeFlag(QListView::ViewMode mode)
{
    if (mode == QListView::IconMode) {
        setIconSize( QSize(170, 170) );
        setGridSize( QSize(170, 170) );
    } else {
        setIconSize( QSize(36, 36) );
        setGridSize( QSize(36, 36) );
    }
    setViewMode(mode);
}

void MusicListDataView::setPlaying(const MetaPtr meta)
{
    Q_D(MusicListDataView);
    d->playing = meta;
    d->musciListDialog->setPlaying(playing());
}

MetaPtr MusicListDataView::playing() const
{
    Q_D(const MusicListDataView);
    return d->playing;
}

void MusicListDataView::showContextMenu(const QPoint &pos, PlaylistPtr selectedPlaylist, PlaylistPtr favPlaylist, QList<PlaylistPtr> newPlaylists)
{
    Q_D(const MusicListDataView);
    d->musciListDialog->showContextMenu(pos, selectedPlaylist, favPlaylist, newPlaylists);
}

PlayMusicTypePtrList MusicListDataView::playMusicTypePtrList() const
{
    Q_D(const MusicListDataView);
    return d->curPlayMusicTypePtrList;
}

void MusicListDataView::setThemeType(int type)
{
    Q_D(MusicListDataView);
    d->themeType = type;
    d->musciListDialog->setThemeType(type);
}

int MusicListDataView::getThemeType() const
{
    Q_D(const MusicListDataView);
    return d->themeType;
}

void MusicListDataView::setPlayPixmap(QPixmap pixmap)
{
    Q_D(MusicListDataView);
    d->playingPixmap = pixmap;
    d->musciListDialog->setPlayPixmap(pixmap);
    update();
}

QPixmap MusicListDataView::getPlayPixmap() const
{
    Q_D(const MusicListDataView);
    return d->playingPixmap;
}

void MusicListDataView::updateList()
{
    Q_D(MusicListDataView);
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
            d->curPlayMusicTypePtrList.removeAt(i);
            d->model->removeRow(i);
        }
    }
}

void MusicListDataView::mouseMoveEvent(QMouseEvent *event)
{

}

void MusicListDataView::onMusiclistChanged(PlaylistPtr playlist)
{
    Q_D(MusicListDataView);

    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }

    d->model->removeRows(0, d->model->rowCount());

    QString searchStr = playlist->searchStr();
    bool chineseFlag = false;
    for (auto ch : searchStr) {
        if (DMusic::PinyinSearch::isChinese(ch)) {
            chineseFlag = true;
            break;
        }
    }
    d->curPlayMusicTypePtrList.clear();
    for (auto meta : playlist->playMusicTypePtrList()) {
        if (searchStr.isEmpty()) {
            d->addPlayMusicTypePtr(meta);
            d->curPlayMusicTypePtrList.append(meta);
        } else {
            if (chineseFlag) {
                if (meta->name.contains(searchStr, Qt::CaseInsensitive)) {
                    d->addPlayMusicTypePtr(meta);
                    d->curPlayMusicTypePtrList.append(meta);
                }
            } else {
                if (playlist->searchStr().size() == 1) {
                    auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(meta->name);
                    if (!curTextList.isEmpty() && curTextList.first().contains(searchStr, Qt::CaseInsensitive)) {
                        d->addPlayMusicTypePtr(meta);
                        d->curPlayMusicTypePtrList.append(meta);
                    }
                } else {
                    auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(meta->name);
                    if (!curTextList.isEmpty() && curTextList.join("").contains(searchStr, Qt::CaseInsensitive)) {
                        d->addPlayMusicTypePtr(meta);
                        d->curPlayMusicTypePtrList.append(meta);
                    }
                }
            }
        }
    }

    d->model->setPlaylist(playlist);
    updateScrollbar();
}
