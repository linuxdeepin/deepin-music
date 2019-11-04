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

#include "musiclistinfoview.h"

#include <QDebug>
#include <DMenu>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QStyleFactory>
#include <QResizeEvent>
#include <QStandardItemModel>

#include <DDialog>
#include <DDesktopServices>
#include <DScrollBar>

#include "../../core/metasearchservice.h"
#include "../helper/widgethellper.h"

#include "delegate/musicinfoitemdelegate.h"
#include "model/musiclistinfomodel.h"

DWIDGET_USE_NAMESPACE

class MusicListInfoViewPrivate
{
public:
    MusicListInfoViewPrivate(MusicListInfoView *parent): q_ptr(parent) {}

    void addMedia(const MetaPtr meta);
    void removeSelection(QItemSelectionModel *selection);

    MusiclistInfomodel      *model        = nullptr;
    MusicInfoItemDelegate   *delegate     = nullptr;
    int                     themeType     = 1;

    QString                  curName      = "";

    MetaPtr                 playing       = nullptr;
    QPixmap                 playingPixmap = QPixmap(":/mpimage/light/music1.svg");

    MusicListInfoView *q_ptr;
    Q_DECLARE_PUBLIC(MusicListInfoView)
};

MusicListInfoView::MusicListInfoView(QWidget *parent)
    : QListView(parent), d_ptr(new MusicListInfoViewPrivate(this))
{
    Q_D(MusicListInfoView);

    setAutoFillBackground(true);
    auto palette = this->palette();
    QColor BackgroundColor("#FFFFFF");
    palette.setColor(DPalette::Background, BackgroundColor);
    setPalette(palette);
    setIconSize( QSize(36, 36) );
//    setGridSize( QSize(36, 36) );

    d->model = new MusiclistInfomodel(0, 1, this);
    setModel(d->model);

    d->delegate = new MusicInfoItemDelegate;
    setItemDelegate(d->delegate);

    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropOverwriteMode(false);
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropMode(QAbstractItemView::InternalMove);
    setMovement(QListView::Free);

    setSelectionMode(QListView::ExtendedSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MusicListInfoView::customContextMenuRequested,
            this, &MusicListInfoView::requestCustomContextMenu);

    connect(this, &MusicListInfoView::doubleClicked,
    this, [ = ](const QModelIndex & index) {
        MetaPtr meta = d->model->meta(index);
        if (meta == playlist()->playing()) {
            Q_EMIT resume(meta);
        } else {
            Q_EMIT playMedia(meta);
        }
    });
}

MusicListInfoView::~MusicListInfoView()
{

}

MetaPtr MusicListInfoView::activingMeta() const
{
    Q_D(const MusicListInfoView);

    if (d->model->playlist().isNull()) {
        return MetaPtr();
    }

    return d->model->playlist()->playing();
}

MetaPtr MusicListInfoView::firstMeta() const
{
    Q_D(const MusicListInfoView);
    MetaPtr curMeta = nullptr;
    for (int i = 0; i < d->model->rowCount(); ++i) {
        auto index = d->model->index(i, 0);
        MetaPtr meta = d->model->meta(index);
        if (!meta->invalid) {
            curMeta = meta;
            break;
        }
    }
    return curMeta;
}

PlaylistPtr MusicListInfoView::playlist() const
{
    Q_D(const MusicListInfoView);
    return d->model->playlist();
}

QModelIndex MusicListInfoView::findIndex(const MetaPtr meta)
{
    Q_ASSERT(!meta.isNull());
    Q_D(MusicListInfoView);

    return d->model->findIndex(meta);
}

QString MusicListInfoView::curName() const
{
    Q_D(const MusicListInfoView);
    return d->curName;
}

void MusicListInfoView::setPlaying(const MetaPtr meta)
{
    Q_D(MusicListInfoView);
    d->playing = meta;
    update();
}

MetaPtr MusicListInfoView::playing() const
{
    Q_D(const MusicListInfoView);
    return d->playing;
}

void MusicListInfoView::setThemeType(int type)
{
    Q_D(MusicListInfoView);
    d->themeType = type;
}

int MusicListInfoView::getThemeType() const
{
    Q_D(const MusicListInfoView);
    return d->themeType;
}

void MusicListInfoView::setPlayPixmap(QPixmap pixmap)
{
    Q_D(MusicListInfoView);
    d->playingPixmap = pixmap;
    update();
}

QPixmap MusicListInfoView::getPlayPixmap() const
{
    Q_D(const MusicListInfoView);
    return d->playingPixmap;
}

void MusicListInfoView::onMusicListRemoved(const MetaPtrList metalist)
{
    Q_D(MusicListInfoView);

    setAutoScroll(false);
    for (auto meta : metalist) {
        if (meta.isNull()) {
            continue;
        }

        for (int i = 0; i < d->model->rowCount(); ++i) {
            auto index = d->model->index(i, 0);
            auto itemHash = d->model->data(index).toString();
            if (itemHash == meta->hash) {
                d->model->removeRow(i);
            }
        }
    }
    //updateScrollbar();
    setAutoScroll(true);
}

void MusicListInfoView::onMusicError(const MetaPtr meta, int /*error*/)
{
    Q_ASSERT(!meta.isNull());
//    Q_D(MusicListInfoView);

//    qDebug() << error;
//    QModelIndex index = findIndex(meta);

//    auto indexData = index.data().value<MetaPtr>();
//    indexData.invalid = (error != 0);
//    d->m_model->setData(index, QVariant::fromValue<MetaPtr>(indexData));

    update();
}

void MusicListInfoView::onMusicListAdded(const MetaPtrList metalist)
{
    Q_D(MusicListInfoView);
    for (auto meta : metalist) {
        d->addMedia(meta);
    }
}

void MusicListInfoView::onLocate(const MetaPtr meta)
{
    QModelIndex index = findIndex(meta);
    if (!index.isValid()) {
        return;
    }

    clearSelection();

    auto viewRect = QRect(QPoint(0, 0), size());
    if (!viewRect.intersects(visualRect(index))) {
        scrollTo(index, MusicListInfoView::PositionAtCenter);
    }
    setCurrentIndex(index);
}

void MusicListInfoView::onMusiclistChanged(PlaylistPtr playlist, const QString name)
{
    Q_D(MusicListInfoView);

    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }

    d->curName     = name;

    d->model->removeRows(0, d->model->rowCount());
    for (auto TypePtr : playlist->playMusicTypePtrList()) {
        if (TypePtr->name == name) {
            for (auto meta : TypePtr->playlistMeta.metas) {
                d->addMedia(meta);
            }
        }
    }

    d->model->setPlaylist(playlist);
    //updateScrollbar();
}

void MusicListInfoView::keyPressEvent(QKeyEvent *event)
{
    Q_D(MusicListInfoView);
    switch (event->modifiers()) {
    case Qt::NoModifier:
        switch (event->key()) {
        case Qt::Key_Delete:
            QItemSelectionModel *selection = this->selectionModel();
            d->removeSelection(selection);
            break;
        }
        break;
    case Qt::ShiftModifier:
        switch (event->key()) {
        case Qt::Key_Delete:
            break;
        }
        break;
    default:
        break;
    }

    QAbstractItemView::keyPressEvent(event);
}

void MusicListInfoView::keyboardSearch(const QString &search)
{
    Q_UNUSED(search);
// Disable keyborad serach
//    qDebug() << search;
//    QAbstractItemView::keyboardSearch(search);
}

void MusicListInfoViewPrivate::addMedia(const MetaPtr meta)
{
    QStandardItem *newItem = new QStandardItem;
    QPixmap cover(":/common/image/cover_max.svg");
    auto coverData = meta->getCoverData();
    if (coverData.length() > 0) {
        cover = QPixmap::fromImage(QImage::fromData(coverData));
    }
    QIcon icon = QIcon(cover);
    newItem->setIcon(icon);
    model->appendRow(newItem);

    auto row = model->rowCount() - 1;
    QModelIndex index = model->index(row, 0, QModelIndex());
    model->setData(index, meta->hash);
}

void MusicListInfoViewPrivate::removeSelection(QItemSelectionModel *selection)
{
    Q_ASSERT(selection != nullptr);
    Q_Q(MusicListInfoView);

    MetaPtrList metalist;
    for (auto index : selection->selectedRows()) {
        auto meta = model->meta(index);
        metalist << meta;
    }
    Q_EMIT q->removeMusicList(metalist);
}

void MusicListInfoView::showContextMenu(const QPoint &pos,
                                        PlaylistPtr selectedPlaylist,
                                        PlaylistPtr favPlaylist,
                                        QList<PlaylistPtr> newPlaylists)
{
    Q_D(MusicListInfoView);
    QItemSelectionModel *selection = this->selectionModel();

    if (selection->selectedRows().length() <= 0) {
        return;
    }

    QPoint globalPos = this->mapToGlobal(pos);

    DMenu playlistMenu;
    auto newvar = QVariant::fromValue(PlaylistPtr());

    if (selectedPlaylist != favPlaylist) {
        auto act = playlistMenu.addAction(favPlaylist->displayName());
        act->setData(QVariant::fromValue(favPlaylist));
    }

    for (auto playlist : newPlaylists) {
        QFont font(playlistMenu.font());
        QFontMetrics fm(font);
        auto text = fm.elidedText(QString(playlist->displayName().replace("&", "&&")),
                                  Qt::ElideMiddle, 160);
        auto act = playlistMenu.addAction(text);
        act->setData(QVariant::fromValue(playlist));
    }

    playlistMenu.addSeparator();

    auto createPlaylist = playlistMenu.addAction(tr("New playlist"));
    auto font = createPlaylist->font();
    font.setWeight(QFont::DemiBold);
    createPlaylist->setFont(font);
    createPlaylist->setData(newvar);

    connect(&playlistMenu, &DMenu::triggered, this, [ = ](QAction * action) {
        auto playlist = action->data().value<PlaylistPtr >();
        qDebug() << playlist;
        MetaPtrList metalist;
        for (auto &index : selection->selectedRows()) {
            auto meta = d->model->meta(index);
            if (!meta.isNull()) {
                metalist << meta;
            }
        }
        Q_EMIT addToPlaylist(playlist, metalist);
    });

    bool singleSelect = (1 == selection->selectedRows().length());

    DMenu myMenu;
    QAction *playAction = nullptr;
    QAction *pauseAction = nullptr;
    if (singleSelect) {
        auto meta = d->model->meta(selection->selectedRows().first());
        if (d->model->playlist()->playingStatus() && d->playing == meta) {
            pauseAction = myMenu.addAction(tr("Pause"));
        } else {
            playAction = myMenu.addAction(tr("Play"));
        }
    }
    myMenu.addAction(tr("Add to playlist"))->setMenu(&playlistMenu);
    myMenu.addSeparator();

    QAction *displayAction = nullptr;
    if (singleSelect) {
        displayAction = myMenu.addAction(tr("Display in file manager"));
    }

    auto removeAction = myMenu.addAction(tr("Remove from playlist"));
    auto deleteAction = myMenu.addAction(tr("Delete from local disk"));

    QAction *songAction = nullptr;

    DMenu textCodecMenu;
    if (singleSelect) {
        auto index = selection->selectedRows().first();
        auto meta = d->model->meta(index);
        QList<QByteArray> codecList = DMusic::detectMetaEncodings(meta);
//        codecList << "utf-8" ;
        for (auto codec : codecList) {
            auto act = textCodecMenu.addAction(codec);
            act->setData(QVariant::fromValue(codec));
        }

//        if (codecList.length() > 1) {
//            myMenu.addSeparator();
//            myMenu.addAction(tr("Encoding"))->setMenu(&textCodecMenu);
//        }

        myMenu.addSeparator();
        songAction = myMenu.addAction(tr("Song info"));

        connect(&textCodecMenu, &DMenu::triggered, this, [ = ](QAction * action) {
            auto codec = action->data().toByteArray();
            meta->updateCodec(codec);
            Q_EMIT updateMetaCodec(meta);
        });
    }

    if (playAction) {
        connect(playAction, &QAction::triggered, this, [ = ](bool) {
            auto index = selection->selectedRows().first();
            if (d->model->meta(index) == playlist()->playing()) {
                Q_EMIT resume(d->model->meta(index));
            } else {
                Q_EMIT playMedia(d->model->meta(index));
            }
        });
    }

    if (pauseAction) {
        connect(pauseAction, &QAction::triggered, this, [ = ](bool) {
            auto index = selection->selectedRows().first();
            Q_EMIT pause(d->model->meta(index));
        });
    }

    if (displayAction) {
        connect(displayAction, &QAction::triggered, this, [ = ](bool) {
            auto index = selection->selectedRows().first();
            auto meta = d->model->meta(index);
            auto dirUrl = QUrl::fromLocalFile(meta->localPath);
            Dtk::Widget::DDesktopServices::showFileItem(dirUrl);
        });
    }

    if (removeAction) {
        connect(removeAction, &QAction::triggered, this, [ = ](bool) {
            d->removeSelection(selection);
        });
    }

    if (deleteAction) {
        connect(deleteAction, &QAction::triggered, this, [ = ](bool) {
            bool containsCue = false;
            MetaPtrList metalist;
            for (auto index : selection->selectedRows()) {
                auto meta = d->model->meta(index);
                if (!meta->cuePath.isEmpty()) {
                    containsCue = true;
                }
                metalist << meta;
            }

            Dtk::Widget::DDialog warnDlg(this);
            warnDlg.setTextFormat(Qt::RichText);
            warnDlg.addButton(tr("Cancel"), true, Dtk::Widget::DDialog::ButtonWarning);
            warnDlg.addButton(tr("Delete"), false, Dtk::Widget::DDialog::ButtonNormal);

            auto cover = QImage(QString(":/common/image/del_notify.svg"));
            if (1 == metalist.length()) {
                auto meta = metalist.first();
                auto coverData = MetaSearchService::coverData(meta);
                if (coverData.length() > 0) {
                    cover = QImage::fromData(coverData);
                }
                warnDlg.setMessage(QString(tr("Are you sure you want to delete %1?")).arg(meta->title));
            } else {
                warnDlg.setMessage(QString(tr("Are you sure you want to delete the selected %1 songs?")).arg(metalist.length()));
            }

            if (containsCue) {
                warnDlg.setTitle(tr("Are you sure you want to delete the selected %1 songs?").arg(metalist.length()));
                warnDlg.setMessage(tr("Deleting the current song will also delete the song files contained"));
            }
            auto coverPixmap =  QPixmap::fromImage(WidgetHelper::cropRect(cover, QSize(64, 64)));

            warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
            if (1 == warnDlg.exec()) {
                Q_EMIT deleteMusicList(metalist);
            }
        });
    }

    if (songAction) {
        connect(songAction, &QAction::triggered, this, [ = ](bool) {
            auto index = selection->selectedRows().first();
            auto meta = d->model->meta(index);
            Q_EMIT showInfoDialog(meta);
        });
    }

    myMenu.exec(globalPos);
}

void MusicListInfoView::dragEnterEvent(QDragEnterEvent *event)
{
    QListView::dragEnterEvent(event);
}

void MusicListInfoView::startDrag(Qt::DropActions supportedActions)
{
    Q_D(MusicListInfoView);

    MetaPtrList list;
    for (auto index : selectionModel()->selectedIndexes()) {
        list << d->model->meta(index);
    }

    setAutoScroll(false);
    QListView::startDrag(supportedActions);
    setAutoScroll(true);

    QMap<QString, int> hashIndexs;
    for (int i = 0; i < d->model->rowCount(); ++i) {
        auto index = d->model->index(i, 0);
        auto hash = d->model->data(index).toString();
        Q_ASSERT(!hash.isEmpty());
        hashIndexs.insert(hash, i);
    }
    d->model->playlist()->saveSort(hashIndexs);
    Q_EMIT customSort();

    QItemSelection selection;
    for (auto meta : list) {
        if (!meta.isNull()) {
            auto index = this->findIndex(meta);
            selection.append(QItemSelectionRange(index));
        }
    }
    if (!selection.isEmpty()) {
        selectionModel()->select(selection, QItemSelectionModel::Select);
    }
}

