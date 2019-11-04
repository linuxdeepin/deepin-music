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

#include <DMenu>
#include <DDialog>
#include <DScrollBar>
#include <DPalette>
#include <DApplicationHelper>

#include "musiclistviewitem.h"

DGUI_USE_NAMESPACE

MusicListView::MusicListView(QWidget *parent) : DListView(parent)
{
    model = new QStandardItemModel(this);
    setModel(model);
    delegate = new DStyledItemDelegate(this);
    //delegate->setBackgroundType(DStyledItemDelegate::NoBackground);
    auto delegateMargins = delegate->margins();
    delegateMargins.setLeft(18);
    delegate->setMargins(delegateMargins);
    setItemDelegate(delegate);

    playingPixmap = QPixmap(":/mpimage/light/music1.svg");

    auto font = this->font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    font.setPixelSize(14);
    setFont(font);

    setIconSize( QSize(20, 20) );
    setGridSize( QSize(40, 40) );
    setItemSize(QSize(40, 40));

    setFrameShape(QFrame::NoFrame);

    DPalette pa = DApplicationHelper::instance()->palette(this);
    pa.setColor(DPalette::ItemBackground, Qt::transparent);
    DApplicationHelper::instance()->setPalette(this, pa);

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
    this, [ = ](QStandardItem * item) {
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
                }
            }
        }
    });
    connect(this, &MusicListView::currentChanged,
    this, [ = ](const QModelIndex & current, const QModelIndex & previous) {
        if (current.row() < 0 || current.row() >= allPlaylists.size()) {
            this->clearSelected();
            return ;
        }

        auto playlistPtr = allPlaylists[current.row()];
        QString rStr;
        if (m_type == 1) {
            rStr = "light";
        } else {
            rStr = "dark";
        }
        for (int i = 0; i < model->rowCount(); i++) {
            auto curIndex = model->index(i, 0);
            auto curStandardItem = dynamic_cast<DStandardItem *>(model->itemFromIndex(curIndex));
            auto curItemRow = curStandardItem->row();
            if (curItemRow < 0 || curItemRow >= allPlaylists.size())
                continue;
            auto playlist = allPlaylists[curItemRow];
            QString typeStr;
            if (current == curIndex) {
                typeStr = "active";
            } else {
                typeStr = "normal";
            }
            QIcon icon = QIcon(QString(":/mpimage/%1/%2/famous_ballad_%2.svg").arg(rStr).arg(typeStr));
            if (playlist->id() == AlbumMusicListID) {
                icon = QIcon(QString(":/mpimage/%1/%2/album_%2.svg").arg(rStr).arg(typeStr));
            } else if (playlist->id() == ArtistMusicListID) {
                icon = QIcon(QString(":/mpimage/%1/%2/singer_%2.svg").arg(rStr).arg(typeStr));
            } else if (playlist->id() == AllMusicListID) {
                icon = QIcon(QString(":/mpimage/%1/%2/all_music_%2.svg").arg(rStr).arg(typeStr));
            } else if (playlist->id() == FavMusicListID) {
                icon = QIcon(QString(":/mpimage/%1/%2/my_collection_%2.svg").arg(rStr).arg(typeStr));
            } else {
                icon = QIcon(QString(":/mpimage/%1/%2/famous_ballad_%2.svg").arg(rStr).arg(typeStr));
            }
            curStandardItem->setIcon(icon);
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
        icon = QIcon(QString(":/mpimage/%1/normal/album_normal.svg").arg(rStr));
    } else if (playlist->id() == ArtistMusicListID) {
        icon = QIcon(QString(":/mpimage/%1/normal/singer_normal.svg").arg(rStr));
    } else if (playlist->id() == AllMusicListID) {
        icon = QIcon(QString(":/mpimage/%1/normal/all_music_normal.svg").arg(rStr));
    } else if (playlist->id() == FavMusicListID) {
        icon = QIcon(QString(":/mpimage/%1/normal/my_collection_normal.svg").arg(rStr));
    } else {
        icon = QIcon(QString(":/mpimage/%1/normal/famous_ballad_normal.svg").arg(rStr));
    }

    allPlaylists.append(playlist);
    auto item = new DStandardItem(icon, playlist->displayName());
    if (m_type == 1) {
        item->setForeground(QColor("#414D68"));
    } else {
        item->setForeground(QColor("#C0C6D4"));
    }
    model->appendRow(item);
    if (addFlag) {
        edit(model->index(item->row(), 0));
        setCurrentItem(item);
        scrollToBottom();
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
        QIcon playingIcon(playingPixmap);
        playingIcon.actualSize(QSize(20, 20));

        DViewItemActionList itemActionList = curItem->actionList(Qt::RightEdge);
        if (!itemActionList.isEmpty()) {
            itemActionList.first()->setIcon(playingIcon);
        } else {
            DViewItemActionList  actionList;
            auto viewItemAction = new DViewItemAction(Qt::AlignCenter);
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
    QString rStr;
    if (m_type == 1) {
        rStr = "light";
    } else {
        rStr = "dark";
    }
    QString typeStr = "normal";
    for (int i = 0; i < model->rowCount(); i++) {
        auto curIndex = model->index(i, 0);
        auto curStandardItem = dynamic_cast<DStandardItem *>(model->itemFromIndex(curIndex));
        auto curItemRow = curStandardItem->row();
        if (curItemRow < 0 || curItemRow >= allPlaylists.size())
            continue;
        auto playlist = allPlaylists[curItemRow];
        QIcon icon = QIcon(QString(":/mpimage/%1/%2/famous_ballad_%2.svg").arg(rStr).arg(typeStr));
        if (playlist->id() == AlbumMusicListID) {
            icon = QIcon(QString(":/mpimage/%1/%2/album_%2.svg").arg(rStr).arg(typeStr));
        } else if (playlist->id() == ArtistMusicListID) {
            icon = QIcon(QString(":/mpimage/%1/%2/singer_%2.svg").arg(rStr).arg(typeStr));
        } else if (playlist->id() == AllMusicListID) {
            icon = QIcon(QString(":/mpimage/%1/%2/all_music_%2.svg").arg(rStr).arg(typeStr));
        } else if (playlist->id() == FavMusicListID) {
            icon = QIcon(QString(":/mpimage/%1/%2/my_collection_%2.svg").arg(rStr).arg(typeStr));
        } else {
            icon = QIcon(QString(":/mpimage/%1/%2/famous_ballad_%2.svg").arg(rStr).arg(typeStr));
        }
        curStandardItem->setIcon(icon);
    }
}

void MusicListView::changePicture(QPixmap pixmap, QPixmap albumPixmap)
{
    this->playingPixmap = pixmap;
    this->albumPixmap = albumPixmap;
    QPixmap curPixmap = pixmap;
//    auto indexes = this->selectedIndexes();
//    if (!indexes.isEmpty() && playingItem != nullptr && indexes[0].row() == playingItem->row()) {
//        curPixmap = albumPixmap;
//    }
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
    }
    update();
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
    for (int i = 0; i < count(); i++) {
        auto item = model->index(i, 0);
        if (this->isPersistentEditorOpen(item))
            closePersistentEditor(item);
    }
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
            warnDlg.addButton(tr("Cancel"), false, Dtk::Widget::DDialog::ButtonNormal);
            warnDlg.addButton(tr("Delete"), true, Dtk::Widget::DDialog::ButtonWarning);
            if (1 == warnDlg.exec()) {
                allPlaylists.removeAt(item->row());
                model->removeRow(item->row());
                //delete model->takeItem(item->row());
                Q_EMIT m_data->removed();
            }
        }
    }
}

void MusicListView::showContextMenu(const QPoint &pos)
{
    // get select
//    auto indexes = this->selectedIndexes();
//    if (indexes.size() != 1) {
//        return;
//    }

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

    DMenu menu;
    QAction *playact = nullptr;
    QAction *pauseact = nullptr;
    if (m_data->playingStatus() && m_data->playing() != nullptr) {
        pauseact = menu.addAction(tr("Pause"));
        pauseact->setDisabled(0 == m_data->length());
    } else {
        playact = menu.addAction(tr("Play"));
        playact->setDisabled(0 == m_data->length());
    }

    if (m_data->id() != AllMusicListID && m_data->id() != AlbumMusicListID &&
            m_data->id() != ArtistMusicListID && m_data->id() != FavMusicListID) {
        menu.addAction(tr("Rename"));
        menu.addAction(tr("Delete"));
    }
    if (m_data->id() == AlbumMusicListID || m_data->id() == ArtistMusicListID) {
        if (playact != nullptr)
            playact->setDisabled(m_data->playMusicTypePtrList().size() == 0);
        if (pauseact != nullptr)
            pauseact->setDisabled(m_data->playMusicTypePtrList().size() == 0);
    }

    connect(&menu, &DMenu::triggered, this, [ = ](QAction * action) {
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
            warnDlg.addButton(tr("Cancel"), false, Dtk::Widget::DDialog::ButtonNormal);
            warnDlg.addButton(tr("Delete"), true, Dtk::Widget::DDialog::ButtonWarning);
            if (1 == warnDlg.exec()) {
                allPlaylists.removeAt(item->row());
                model->removeRow(item->row());
                //delete model->takeItem(item->row());
                Q_EMIT m_data->removed();
            }

        }
    });

    menu.exec(globalPos);
}
void MusicListView::slotTheme(int type)
{
    m_type = type;
    QString rStr;
    if (type == 1) {
        rStr = "light";
    } else {
        rStr = "dark";
    }
    for (int i = 0; i < model->rowCount(); i++) {
        auto current = currentIndex();
        for (int i = 0; i < model->rowCount(); i++) {
            auto curIndex = model->index(i, 0);
            auto curStandardItem = dynamic_cast<DStandardItem *>(model->itemFromIndex(curIndex));
            auto curItemRow = curStandardItem->row();
            if (curItemRow < 0 || curItemRow >= allPlaylists.size())
                continue;
            auto playlist = allPlaylists[curStandardItem->row()];
            QString typeStr;
            if (current == curIndex) {
                typeStr = "active";
            } else {
                typeStr = "normal";
            }
            QIcon icon = QIcon(QString(":/mpimage/%1/%2/famous_ballad_%2.svg").arg(rStr).arg(typeStr));
            if (playlist->id() == AlbumMusicListID) {
                icon = QIcon(QString(":/mpimage/%1/%2/album_%2.svg").arg(rStr).arg(typeStr));
            } else if (playlist->id() == ArtistMusicListID) {
                icon = QIcon(QString(":/mpimage/%1/%2/singer_%2.svg").arg(rStr).arg(typeStr));
            } else if (playlist->id() == AllMusicListID) {
                icon = QIcon(QString(":/mpimage/%1/%2/all_music_%2.svg").arg(rStr).arg(typeStr));
            } else if (playlist->id() == FavMusicListID) {
                icon = QIcon(QString(":/mpimage/%1/%2/my_collection_%2.svg").arg(rStr).arg(typeStr));
            } else {
                icon = QIcon(QString(":/mpimage/%1/%2/famous_ballad_%2.svg").arg(rStr).arg(typeStr));
            }
            curStandardItem->setIcon(icon);
            if (m_type == 1) {
                curStandardItem->setForeground(QColor("#414D68"));
            } else {
                curStandardItem->setForeground(QColor("#C0C6D4"));
            }
        }
    }

}
