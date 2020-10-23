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

#include "musiclistview.h"

#include <QDebug>
#include <QKeyEvent>
#include <QMimeData>

#include <DMenu>
#include <DDialog>
#include <DScrollBar>
#include <DPalette>
#include <DApplicationHelper>

//#include "usiclistviewitem.h"
#include "./model/musiclistmodel.h"
#include "playlistview.h"

DGUI_USE_NAMESPACE

MusicListView::MusicListView(QWidget *parent) : DListView(parent)
{
    model = new MusiclistModel(this);
    setModel(model);
    delegate = new DStyledItemDelegate(this);
    //delegate->setBackgroundType(DStyledItemDelegate::NoBackground);
    auto delegateMargins = delegate->margins();
    delegateMargins.setLeft(18);
    delegate->setMargins(delegateMargins);
    setItemDelegate(delegate);

    setViewportMargins(8, 0, 8, 0);

    playingPixmap = QPixmap(":/mpimage/light/music1.svg");
    albumPixmap = QPixmap(":/mpimage/light/music_withe_sidebar/music1.svg");
    defaultPixmap = QPixmap(":/mpimage/light/music_withe_sidebar/music1.svg");
    auto font = this->font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    font.setPixelSize(14);
    setFont(font);

    setIconSize(QSize(20, 20));
    setItemSize(QSize(40, 40));

    setFrameShape(QFrame::NoFrame);

    DPalette pa = DApplicationHelper::instance()->palette(this);
    pa.setColor(DPalette::ItemBackground, Qt::transparent);
    DApplicationHelper::instance()->setPalette(this, pa);

    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSelectionMode(QListView::SingleSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MusicListView::customContextMenuRequested,
            this, &MusicListView::showContextMenu);

    connect(this, &MusicListView::pressed,
    this, [ = ]() {
        closeAllPersistentEditor();
    });

    connect(model, &QStandardItemModel::itemChanged,
            this, &MusicListView::onRename);
    connect(this, &MusicListView::currentChanged,
    this, [ = ](const QModelIndex & current, const QModelIndex & previous) {
        Q_UNUSED(previous)
        if (current.row() < 0 || current.row() >= allPlaylists.size()) {
            this->clearSelected();
            pixmapState = false;
            return ;
        }
        if (state() != EditingState) {
            auto curStandardItem = dynamic_cast<DStandardItem *>(model->itemFromIndex(current));
            QString name = curStandardItem->data(Qt::UserRole + 10).toString();
            if (name != AlbumMusicListID && name != ArtistMusicListID && name != AllMusicListID && name != FavMusicListID) {
                curStandardItem->setIcon(QIcon::fromTheme("music_famousballad"));
            }
        }

        QPixmap curPixmap = QPixmap(":/mpimage/light/music1.svg");
        auto indexes = this->selectedIndexes();
        if (playingItem != nullptr && current == playingItem->index()) {
            auto mdata = allPlaylists.at(current.row());
            if (mdata->playing() != nullptr) {
                curPixmap = QPixmap(":/mpimage/light/music_withe_sidebar/music1.svg");
                pixmapState = true;
            }
        }
        if (playingItem != nullptr) {
            auto curItem = dynamic_cast<DStandardItem *>(playingItem);
            //delete
            QIcon playingIcon(curPixmap);
            playingIcon.actualSize(QSize(20, 20));
            DViewItemActionList actionList = curItem->actionList(Qt::RightEdge);
            if (!actionList.isEmpty()) {
                actionList.first()->setIcon(playingIcon);
            } else {
                DViewItemActionList  actionList;
                auto viewItemAction = new DViewItemAction(Qt::AlignCenter);
                viewItemAction->setIcon(playingIcon);
                actionList.append(viewItemAction);
                curItem->setActionList(Qt::RightEdge, actionList);

            }
            update();
        }

    });

    connect(this, &MusicListView::triggerEdit,
    this, [ = ](const QModelIndex & index) {
        if (DGuiApplicationHelper::instance()->themeType() == 1) {
            auto curStandardItem = dynamic_cast<DStandardItem *>(model->itemFromIndex(index));
            curStandardItem->setIcon(QIcon(QString(":/mpimage/light/normal/famous_ballad_normal.svg")));
        }
    });
}

MusicListView::~MusicListView()
{

}

void MusicListView::addMusicList(PlaylistPtr playlist, bool addFlag)
{
    if (playlist == nullptr)
        return;
    QString rStr;
    if (m_type == 1) {
        rStr = "light";
    } else {
        rStr = "dark";
    }
    QIcon icon(QString(":/mpimage/%1/normal/famous_ballad_normal.svg").arg(rStr));
    if (playlist->id() == AlbumMusicListID) {
//        icon = QIcon(QString(":/mpimage/%1/normal/album_normal.svg").arg(rStr));
        icon = QIcon::fromTheme("music_album");
    } else if (playlist->id() == ArtistMusicListID) {
//        icon = QIcon(QString(":/mpimage/%1/normal/singer_normal.svg").arg(rStr));
        icon = QIcon::fromTheme("music_singer");
    } else if (playlist->id() == AllMusicListID) {
//        icon = QIcon(QString(":/mpimage/%1/normal/all_music_normal.svg").arg(rStr));
        icon = QIcon::fromTheme("music_allmusic");
    } else if (playlist->id() == FavMusicListID) {
//        icon = QIcon(QString(":/mpimage/%1/normal/my_collection_normal.svg").arg(rStr));
        icon = QIcon::fromTheme("music_mycollection");
    } else {
//        icon = QIcon(QString(":/mpimage/%1/normal/famous_ballad_normal.svg").arg(rStr));
        icon = QIcon::fromTheme("music_famousballad");
    }

    allPlaylists.append(playlist);
    QString displayName = "";
    if (playlist->id() == FavMusicListID) {
        displayName = tr("My Favorites");
    } else {
        displayName = playlist->displayName();
    }
    auto item = new DStandardItem(icon, displayName);

    if (playlist->id() == AlbumMusicListID) {
        item->setData(AlbumMusicListID, Qt::UserRole + 10);
    } else if (playlist->id() == ArtistMusicListID) {
        item->setData(ArtistMusicListID, Qt::UserRole + 10);
    } else if (playlist->id() == AllMusicListID) {
        item->setData(AllMusicListID, Qt::UserRole + 10);
    } else if (playlist->id() == FavMusicListID) {
        item->setData(FavMusicListID, Qt::UserRole + 10);
    }

    auto itemFont = item->font();
    itemFont.setPixelSize(14);
    item->setFont(itemFont);
    if (m_type == 1) {
        item->setForeground(QColor("#414D68"));
    } else {
        item->setForeground(QColor("#C0C6D4"));
    }
    model->appendRow(item);
    adjustHeight();
    if (addFlag) {
        setCurrentItem(item);
        edit(model->index(item->row(), 0));
        scrollToBottom();
        m_sizeChangedFlag = true;
    }
}

QStandardItem *MusicListView::item(int row, int column) const
{
    return model->item(row, column);
}

void MusicListView::setCurrentItem(QStandardItem *item)
{
    setCurrentIndex(model->indexFromItem(item));
}

PlaylistPtr MusicListView::playlistPtr(const QModelIndex &index)
{
    PlaylistPtr ptr = nullptr;
    if (index.row() >= 0 && index.row() < allPlaylists.size())
        ptr = allPlaylists[index.row()];
    return ptr;
}

PlaylistPtr MusicListView::playlistPtr(QStandardItem *item)
{
    PlaylistPtr ptr = nullptr;
    if (item->row() < allPlaylists.size())
        ptr = allPlaylists[item->row()];
    return ptr;
}

void MusicListView::setCurPlaylist(QStandardItem *item)
{
    auto curItem = dynamic_cast<DStandardItem *>(item);
    playingItem = curItem;
    if (curItem) {
        QIcon playingIcon;
        if (pixmapState) {
            QIcon icon(defaultPixmap);
            playingIcon = icon;
        } else {
            QIcon icon(playingPixmap);
            playingIcon = icon;
        }
        playingIcon.actualSize(QSize(20, 20));

        DViewItemActionList itemActionList = curItem->actionList(Qt::RightEdge);
        if (!itemActionList.isEmpty()) {
            itemActionList.first()->setIcon(playingIcon);
        } else {
            DViewItemActionList  actionList;

            /*----------delegate QSize-----------*/
            auto viewItemAction = new DViewItemAction(Qt::AlignCenter, QSize(20, 20));
            viewItemAction->setIcon(playingIcon);
            actionList.append(viewItemAction);
            curItem->setActionList(Qt::RightEdge, actionList);
        }

    }
    DViewItemActionList  clearActionList;
    QIcon playingIcon;
    playingIcon.actualSize(QSize(20, 20));
    auto viewItemAction = new DViewItemAction(Qt::AlignCenter);
    viewItemAction->setIcon(playingIcon);
    clearActionList.append(viewItemAction);
    for (int i = 0; i < model->rowCount(); i++) {
        auto curStandardItem = dynamic_cast<DStandardItem *>(model->itemFromIndex(model->index(i, 0)));
        if (curStandardItem != nullptr && curStandardItem != curItem) {
            DViewItemActionList actionList = curStandardItem->actionList(Qt::RightEdge);
            if (!actionList.isEmpty()) {
                actionList.first()->setIcon(playingIcon);
            }
        }
    }
    //setCurrentItem(item);
    update();
}

void MusicListView::closeAllPersistentEditor()
{
    for (int i = 0; i < model->rowCount(); i++) {
        auto item = model->index(i, 0);
        if (this->isPersistentEditorOpen(item))
            closePersistentEditor(item);
    }
}

void MusicListView::clearSelected()
{
    clearSelection();
    setCurrentItem(nullptr);
    if (playingItem != nullptr && playingItem->index().row() >= 0 && playingItem->index().row() < allPlaylists.size()) {
        auto curItem = dynamic_cast<DStandardItem *>(playingItem);
        if (curItem != nullptr) {
            //delete
            QIcon playingIcon(QPixmap(":/mpimage/light/music1.svg"));
            playingIcon.actualSize(QSize(20, 20));
            DViewItemActionList actionList = curItem->actionList(Qt::RightEdge);
            if (!actionList.isEmpty()) {
                actionList.first()->setIcon(playingIcon);
            } else {
                DViewItemActionList  actionList;
                auto viewItemAction = new DViewItemAction(Qt::AlignCenter);
                viewItemAction->setIcon(playingIcon);
                actionList.append(viewItemAction);
                curItem->setActionList(Qt::RightEdge, actionList);
            }
        }
    }
}

void MusicListView::changePicture(QPixmap pixmap, QPixmap albumPixmap)
{
    this->playingPixmap = pixmap;
    this->albumPixmap = albumPixmap;
    QPixmap curPixmap = pixmap;


    auto indexes = this->selectedIndexes();
    if (!indexes.isEmpty() && playingItem != nullptr) {
        if (indexes[0].row() >= 0 && indexes[0].row() < allPlaylists.count()) {
            auto mdata = allPlaylists.at(indexes[0].row());
            if (mdata->playing() != nullptr)
                curPixmap = albumPixmap;
        }

    }
    if (playingItem != nullptr) {
        auto curItem = dynamic_cast<DStandardItem *>(playingItem);
        //delete
        QIcon playingIcon(curPixmap);
        playingIcon.actualSize(QSize(20, 20));
        DViewItemActionList actionList = curItem->actionList(Qt::RightEdge);
        if (!actionList.isEmpty()) {
            actionList.first()->setIcon(playingIcon);
        } else {
            DViewItemActionList  actionList;
            auto viewItemAction = new DViewItemAction(Qt::AlignCenter);
            viewItemAction->setIcon(playingIcon);
            actionList.append(viewItemAction);
            curItem->setActionList(Qt::RightEdge, actionList);
        }
        update();
    }
}

void MusicListView::adjustHeight()
{
    setMinimumHeight(model->rowCount() * 40);
}

void MusicListView::setSizeChangedFlag(bool flag)
{
    m_sizeChangedFlag = flag;
}

bool MusicListView::getSizeChangedFlag()
{
    return m_sizeChangedFlag;
}

//void MusicListView::startDrag(Qt::DropActions supportedActions)
//{
//    DListWidget::startDrag(supportedActions);
//    qDebug() << "drag end";

//    QStringList uuids;

//    for (int i = 0; i < this->count(); ++i) {
//        QListWidgetItem *item = this->item(i);
//        MusicListViewItem *playlistItem = dynamic_cast<MusicListViewItem *>(item);
//        uuids << playlistItem->data()->id();
//    }
//    Q_EMIT customResort(uuids);
//}

void MusicListView::mousePressEvent(QMouseEvent *event)
{
    //    for (int i = 0; i < count(); i++) {
    //        auto itemIndex = model->index(i, 0);
    //        if (this->isPersistentEditorOpen(itemIndex)) {
    //            auto item = model->itemFromIndex(itemIndex);
    //            onRename(item);
    //            closePersistentEditor(itemIndex);
    //        }
    //    }
    DListView::mousePressEvent(event);
}

void MusicListView::keyPressEvent(QKeyEvent *event)
{
    DListView::keyPressEvent(event);
    if (event->key() == Qt::Key_Delete) {
        auto indexes = this->selectedIndexes();
        if (indexes.size() != 1) {
            return;
        }
        auto item = model->itemFromIndex(indexes.first());
        if (!item) {
            return;
        }
        auto m_data = allPlaylists[item->row()];
        if (!m_data) {
            return;
        }

        if (m_data->id() != AllMusicListID && m_data->id() != AlbumMusicListID &&
                m_data->id() != ArtistMusicListID && m_data->id() != FavMusicListID) {
            QString message = QString(tr("Are you sure you want to delete this playlist?"));

            DDialog warnDlg(this);
            warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
            warnDlg.setTextFormat(Qt::AutoText);
            warnDlg.setTitle(message);
            warnDlg.addSpacing(20);
            warnDlg.addButton(tr("Cancel"), false, Dtk::Widget::DDialog::ButtonNormal);
            warnDlg.addButton(tr("Delete"), true, Dtk::Widget::DDialog::ButtonWarning);
            if (1 == warnDlg.exec()) {
                int t_index = item->row();
                model->removeRow(item->row());
                allPlaylists.removeAt(t_index);
                if (item == playingItem)
                    playingItem = nullptr;

                //delete model->takeItem(item->row());
                Q_EMIT m_data->removed();
                if (m_data->playing() != nullptr || allPlaylists.isEmpty()) {
                    if (m_data->active())
                        Q_EMIT removeAllList(m_data->playing());
                }
                if (allPlaylists.isEmpty())
                    Q_EMIT changeToAllMusic();

                adjustHeight();
            }
        }
    } else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
        auto indexes = this->selectedIndexes();
        if (indexes.size() != 1) {
            return;
        }
        scrollTo(indexes.first());
    }
}

void MusicListView::dragEnterEvent(QDragEnterEvent *event)
{
    auto t_formats = event->mimeData()->formats();
    qDebug() << t_formats;
    if (event->mimeData()->hasFormat("text/uri-list") || event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    }
}

void MusicListView::dragMoveEvent(QDragMoveEvent *event)
{
    auto index = indexAt(event->pos());
    if (index.isValid() && (event->mimeData()->hasFormat("text/uri-list")  || event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    } else {
        DListView::dragMoveEvent(event);
    }
}

void MusicListView::dropEvent(QDropEvent *event)
{
    auto index = indexAt(event->pos());
    if (!index.isValid())
        return;

    auto t_playlistPtr = playlistPtr(index);
    if (t_playlistPtr == nullptr || (!event->mimeData()->hasFormat("text/uri-list") && !event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))) {
        return;
    }

    if (event->mimeData()->hasFormat("text/uri-list")) {
        auto urls = event->mimeData()->urls();
        QStringList localpaths;
        for (auto &url : urls) {
            localpaths << url.toLocalFile();
        }

        if (!localpaths.isEmpty()) {
            Q_EMIT importSelectFiles(t_playlistPtr, localpaths);
        }
    } else {
        auto *source = qobject_cast<PlayListView *>(event->source());
        if (source != nullptr) {
            MetaPtrList metalist;
            for (auto index : source->selectionModel()->selectedIndexes()) {
                if (index.row() >= 0 && index.row() < source->playMetaPtrList().size()) {
                    auto meta = source->playMetaPtrList()[index.row()];
                    metalist.append(meta);
                }
            }

            if (!metalist.isEmpty())
                Q_EMIT addToPlaylist(t_playlistPtr, metalist);
        }
    }

    DListView::dropEvent(event);
}

void MusicListView::showContextMenu(const QPoint &pos)
{
    auto index = indexAt(pos);
    if (!index.isValid())
        return;

    auto item = model->itemFromIndex(index);
    if (!item) {
        return;
    }
    auto m_data = allPlaylists[item->row()];
    if (!m_data) {
        return;
    }

    QPoint globalPos = this->mapToGlobal(pos);

    DMenu *menu = new  DMenu(this) ;
    QAction *playact = nullptr;
    QAction *pauseact = nullptr;

    if (m_data->playingStatus() && m_data->playing() != nullptr) {
        pauseact = menu->addAction(tr("Pause"));
        pauseact->setDisabled(0 == m_data->length());
    } else {
        playact = menu->addAction(tr("Play"));
        playact->setDisabled(0 == m_data->length());
    }

    if (m_data->id() != AllMusicListID && m_data->id() != AlbumMusicListID &&
            m_data->id() != ArtistMusicListID && m_data->id() != FavMusicListID) {
        menu->addAction(tr("Rename"));
        menu->addAction(tr("Delete"));
    }
    if (m_data->id() == AlbumMusicListID || m_data->id() == ArtistMusicListID) {
        if (playact != nullptr)
            playact->setDisabled(m_data->playMusicTypePtrList().size() == 0);
        if (pauseact != nullptr)
            pauseact->setDisabled(m_data->playMusicTypePtrList().size() == 0);
    }

    connect(menu, &DMenu::triggered, this, [ = ](QAction * action) {

        if (action->text() == tr("Play")) {
            Q_EMIT playall(m_data);
        }
        if (action->text() == tr("Pause")) {
            Q_EMIT pause(m_data, m_data->playing());
        }
        if (action->text() == tr("Rename")) {
            edit(index);
        }
        if (action->text() == tr("Delete")) {
            QString message = QString(tr("Are you sure you want to delete this playlist?"));

            DDialog warnDlg(this);
            warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
            warnDlg.setTextFormat(Qt::AutoText);
            warnDlg.setTitle(message);
            warnDlg.addSpacing(20);
            warnDlg.addButton(tr("Cancel"), false, Dtk::Widget::DDialog::ButtonNormal);
            warnDlg.addButton(tr("Delete"), true, Dtk::Widget::DDialog::ButtonWarning);
            if (1 == warnDlg.exec()) {
                int t_index = item->row();
                model->removeRow(item->row());
                allPlaylists.removeAt(t_index);
                if (item == playingItem)
                    playingItem = nullptr;

                //delete model->takeItem(item->row());
                Q_EMIT m_data->removed();
                if (m_data->playing() != nullptr || allPlaylists.isEmpty()) {
                    if (m_data->active())
                        Q_EMIT removeAllList(m_data->playing());
                }
                if (allPlaylists.isEmpty())
                    Q_EMIT changeToAllMusic();

                adjustHeight();
            }
        }
    });

    menu->exec(globalPos);
}
void MusicListView::slotTheme(int type)
{
    m_type = type;

    for (int i = 0; i < model->rowCount(); i++) {
        auto curIndex = model->index(i, 0);
        auto curStandardItem = dynamic_cast<DStandardItem *>(model->itemFromIndex(curIndex));
        auto curItemRow = curStandardItem->row();
        if (curItemRow < 0 || curItemRow >= allPlaylists.size())
            continue;

        if (m_type == 1) {
            curStandardItem->setForeground(QColor("#414D68"));
        } else {
            curStandardItem->setForeground(QColor("#C0C6D4"));
        }
    }
}

void MusicListView::onRename(QStandardItem *item)
{
    auto curItemRow = item->row();
    if (curItemRow < 0 || curItemRow >= allPlaylists.size())
        return;
    auto playlistPtr = allPlaylists[item->row()];
    if (playlistPtr->displayName() != item->text()) {
        if (item->text().isEmpty()) {
            item->setText(playlistPtr->displayName());
        } else {
            bool existFlag = false;
            for (int i = 0; i < count(); i++) {
                auto curItem = model->itemFromIndex(model->index(i, 0));
                if (curItem == item)
                    continue;
                if (item->text() == curItem->text()) {
                    existFlag = true;
                }
            }
            if (existFlag) {
                item->setText(playlistPtr->displayName());
            } else {
                playlistPtr->setDisplayName(item->text());
                Q_EMIT playlistPtr->displayNameChanged(item->text());
                Q_EMIT displayNameChanged();
                auto curStandardItem = dynamic_cast<DStandardItem *>(item);
                curStandardItem->setIcon(QIcon::fromTheme("music_famousballad"));
            }
        }
    }
}

void MusicListView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    DListView::closeEditor(editor, hint);
    auto current = currentIndex();
    if (current.row() < 0 || current.row() >= allPlaylists.size()) {
        this->clearSelected();
        return ;
    }

    auto curStandardItem = dynamic_cast<DStandardItem *>(model->itemFromIndex(current));
    curStandardItem->setIcon(QIcon::fromTheme("music_famousballad"));
}
