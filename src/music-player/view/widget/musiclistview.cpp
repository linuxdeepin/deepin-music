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
#include <DMenu>
#include <DDialog>

#include <DScrollBar>
#include <DPalette>

#include "musiclistviewitem.h"

DGUI_USE_NAMESPACE

MusicListView::MusicListView(QWidget *parent) : DListWidget(parent)
{
    setIconSize( QSize(40, 40) );
    setGridSize( QSize(40, 40) );

    setFrameShape(QFrame::NoFrame);
    setAutoFillBackground(true);
    auto pl = palette();
    pl.setColor(DPalette::Base, QColor(Qt::transparent));
    setPalette(pl);

    setSelectionMode(QListView::SingleSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MusicListView::customContextMenuRequested,
            this, &MusicListView::showContextMenu);

    connect(this, &MusicListView::itemSelectionChanged,
    this, [ = ]() {
        for (int i = 0; i < count(); i++) {
            QListWidgetItem *item = this->item(i);
            if (this->isPersistentEditorOpen(item))
                closePersistentEditor(item);
        }
    });

    connect(this, &MusicListView::itemChanged,
    this, [ = ](QListWidgetItem * item) {
        MusicListViewItem *playlistItem = dynamic_cast<MusicListViewItem *>(item);
        if (playlistItem->data()->displayName() != playlistItem->text()) {
            if (playlistItem->text().isEmpty()) {
                playlistItem->setText(playlistItem->data()->displayName());
            } else {
                bool existFlag = false;
                for (int i = 0; i < count(); i++) {
                    QListWidgetItem *curItem = this->item(i);
                    if (curItem == item)
                        continue;
                    if (playlistItem->text() == curItem->text()) {
                        existFlag = true;
                    }
                }
                if (existFlag) {
                    playlistItem->setText(playlistItem->data()->displayName());
                } else {
                    playlistItem->data()->setDisplayName(playlistItem->text());
                    Q_EMIT playlistItem->data()->displayNameChanged(playlistItem->text());
                    Q_EMIT displayNameChanged();
                }
            }
        }
    });
}

MusicListView::~MusicListView()
{

}

void MusicListView::startDrag(Qt::DropActions supportedActions)
{
    DListWidget::startDrag(supportedActions);
    qDebug() << "drag end";

    QStringList uuids;

    for (int i = 0; i < this->count(); ++i) {
        QListWidgetItem *item = this->item(i);
        MusicListViewItem *playlistItem = dynamic_cast<MusicListViewItem *>(item);
        uuids << playlistItem->data()->id();
    }
    Q_EMIT customResort(uuids);
}

void MusicListView::mousePressEvent(QMouseEvent *event)
{
    for (int i = 0; i < count(); i++) {
        QListWidgetItem *item = this->item(i);
        if (this->isPersistentEditorOpen(item))
            closePersistentEditor(item);
    }
    DListWidget::mousePressEvent(event);
}

void MusicListView::showContextMenu(const QPoint &pos)
{
    // get select
    auto items = this->selectedItems();
    if (items.length() != 1) {
        return;
    }

    MusicListViewItem *item = dynamic_cast<MusicListViewItem *>(items.first());
    if (!item) {
        return;
    }
    auto m_data = item->data();
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
        playact->setDisabled(m_data->playMusicTypePtrList().size() == 0);
    }

    connect(&menu, &DMenu::triggered, this, [ = ](QAction * action) {
        if (action->text() == tr("Play")) {
            Q_EMIT playall(m_data);
        }
        if (action->text() == tr("Pause")) {
            Q_EMIT pause(m_data, m_data->playing());
        }
        if (action->text() == tr("Rename")) {
            openPersistentEditor(item);
        }
        if (action->text() == tr("Delete")) {
            QString message = QString(tr("Are you sure you want to delete this playlist?"));

            DDialog warnDlg(this);
            warnDlg.setIcon(QIcon(":/common/image/del_notify.svg"));
            warnDlg.setTextFormat(Qt::AutoText);
            warnDlg.setTitle(message);
            warnDlg.addButton(tr("Cancel"), false, Dtk::Widget::DDialog::ButtonNormal);
            warnDlg.addButton(tr("Delete"), true, Dtk::Widget::DDialog::ButtonWarning);
            if (0 != warnDlg.exec()) {
                delete takeItem(row(item));
                Q_EMIT m_data->removed();
            }

        }
    });

    menu.exec(globalPos);
}
