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
#include <QVBoxLayout>

#include <DDialog>
#include <DDesktopServices>
#include <DScrollBar>
#include <DLabel>

#include "infodialog.h"
#include "player.h"
#include "global.h"
#include "databaseservice.h"

DWIDGET_USE_NAMESPACE

MusicListInfoView::MusicListInfoView(const QString &hash, QWidget *parent)
    : QListView(parent)
    , m_hash(hash)
{
    setFrameShape(QFrame::NoFrame);

    setAutoFillBackground(true);
    auto palette = this->palette();
    QColor BackgroundColor("#FFFFFF");
    palette.setColor(DPalette::Background, BackgroundColor);
    setPalette(palette);
    setIconSize(QSize(36, 36));
    //    setGridSize( QSize(36, 36) );
    m_model = new MusiclistInfomodel(0, 1, this);
    setModel(m_model);

    delegate = new MusicInfoItemDelegate;
    setItemDelegate(delegate);

    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropOverwriteMode(false);
    //setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    //setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Free);
    setLayoutMode(QListView::Batched);

    setSelectionMode(QListView::ExtendedSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MusicListInfoView::customContextMenuRequested,
            this, &MusicListInfoView::showContextMenu);

    connect(this, &MusicListInfoView::doubleClicked, this, &MusicListInfoView::onDoubleClicked);

    connect(Player::instance(), SIGNAL(signalUpdatePlayingIcon()),
            this, SLOT(slotUpdatePlayingIcon()), Qt::DirectConnection);
}

MusicListInfoView::~MusicListInfoView()
{

}

//MetaPtr MusicListInfoView::activingMeta() const
//{
//    Q_D(const MusicListInfoView);

//    if (d->model->playlist().isNull()) {
//        return MetaPtr();
//    }

//    return d->model->playlist()->playing();
//}

//MetaPtr MusicListInfoView::firstMeta() const
//{
//    MetaPtr curMeta = nullptr;
//    for (int i = 0; i < m_model->rowCount(); ++i) {
//        auto index = m_model->index(i, 0);
//        MetaPtr meta = m_model->meta(index);
//        if (!meta->invalid) {
//            curMeta = meta;
//            break;
//        } else {
//            if (QFile::exists(meta->localPath)) {
//                curMeta = meta;
//                break;
//            }
//        }
//    }
//    return curMeta;
//}

//PlaylistPtr MusicListInfoView::playlist() const
//{
//    Q_D(const MusicListInfoView);
//    return d->model->playlist();
//}


//QString MusicListInfoView::curName() const
//{
//    Q_D(const MusicListInfoView);
//    return d->curName;
//}

//MetaPtr MusicListInfoView::playing() const
//{
//    return playing;
//}

void MusicListInfoView::setThemeType(int type)
{
    themeType = type;
}

int MusicListInfoView::getThemeType() const
{
    return themeType;
}

void MusicListInfoView::setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap)
{
    playingPixmap = pixmap;
    sidebarPixmap = sidebarPixmap;
//    auto index = d->model->findIndex(d->playing);
//    if (index.isValid())
//        update(index);

    viewport()->update();
}

QPixmap MusicListInfoView::getSidebarPixmap() const
{
    return sidebarPixmap;
}

QPixmap MusicListInfoView::getPlayPixmap(bool isSelect)
{
    QPixmap playingPixmap = QPixmap(QSize(20, 20));
    playingPixmap.fill(Qt::transparent);
    QPainter painter(&playingPixmap);
    DTK_NAMESPACE::Gui::DPalette pa;// = this->palette();
    if (isSelect) {
        painter.setPen(QColor(Qt::white));
    } else {
        painter.setPen(pa.color(QPalette::Active, DTK_NAMESPACE::Gui::DPalette::Highlight));
    }
    Player::instance()->playingIcon().paint(&painter, QRect(0, 0, 20, 20), Qt::AlignCenter, QIcon::Active, QIcon::On);
    update();
    return playingPixmap;
}

QList<MediaMeta> MusicListInfoView::getMusicListData() const
{
    QList<MediaMeta> musicList;
    QStringList allMetaNames;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        auto index = m_model->index(i, 0);
        MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();
        musicList.append(meta);
    }

    return musicList;
}

void MusicListInfoView::slotUpdatePlayingIcon()
{
    this->viewport()->update();
}

void MusicListInfoView::setMusicListView(QMap<QString, MediaMeta> musicinfos)
{
    m_model->removeRows(0, m_model->rowCount());
    for (auto item : musicinfos) {
        addMedia(item);
    }
}

void MusicListInfoView::showContextMenu(const QPoint &pos)
{
    QItemSelectionModel *selection = this->selectionModel();

    if (selection->selectedRows().length() <= 0) {
        return;
    }

    QPoint globalPos = this->mapToGlobal(pos);
    m_currMeta = this->currentIndex().data(Qt::UserRole).value<MediaMeta>();

    // 添加到播放列表菜单
    DMenu playListMenu;
    connect(&playListMenu, &DMenu::triggered, this, &MusicListInfoView::slotPlayListMenuClicked);

    QAction *actFav = playListMenu.addAction(tr("My favorites"));
    actFav->setData("fav");

    if (DataBaseService::getInstance()->favoriteExist(m_currMeta)) {
        actFav->setEnabled(false);
    } else {
        actFav->setEnabled(true);
    }

    playListMenu.addSeparator();
    playListMenu.addAction(tr("Add to new playlist"))->setData("song list");
    playListMenu.addSeparator();

    // 添加自定义歌单到Menu
    QList<DataBaseService::PlaylistData> strplaylist = DataBaseService::getInstance()->getCustomSongList();
    for (DataBaseService::PlaylistData pd : strplaylist) {
        QAction *pact = playListMenu.addAction(pd.displayName);
//        pact->setData(QVariant(pd.uuid));
//        connect(pact, SIGNAL(triggered()), this, SLOT(slotAddToCustomSongList()));
    }


    DMenu mainMenu;
    QAction *playAction = nullptr;
    QAction *pauseAction = nullptr;

    if (m_currMeta.hash == Player::instance()->activeMeta().hash && Player::instance()->status() == Player::Playing) {
        pauseAction = mainMenu.addAction(tr("Pause"));
    } else {
        playAction = mainMenu.addAction(tr("Play"));
    }

    mainMenu.addAction(tr("Add to playlist"))->setMenu(&playListMenu);
    mainMenu.addSeparator();

    QAction *fileManagementShowAction = nullptr;
    fileManagementShowAction = mainMenu.addAction(tr("Display in file manager"));

    // 从歌单删除
    auto removeSongListAction = mainMenu.addAction(tr("Remove from playlist"));
    // 从本地删除
    auto deleteLocalAction = mainMenu.addAction(tr("Delete from local disk"));

    mainMenu.addSeparator();
    QAction *musicInfoAction = mainMenu.addAction(tr("Song info"));

    if (playAction) {
        connect(playAction, &QAction::triggered, this, &MusicListInfoView::slotPlayActionClicked);
    }

    if (pauseAction) {
        connect(pauseAction, &QAction::triggered, this, &MusicListInfoView::slotPauseActionClicked);
    }

    if (fileManagementShowAction) {
        connect(fileManagementShowAction, &QAction::triggered, this, &MusicListInfoView::slotFileManagementShowClicked);
    }

    if (musicInfoAction) {
        connect(musicInfoAction, &QAction::triggered, this, &MusicListInfoView::slotMusicInfoActionClicked);
    }

    connect(removeSongListAction, &QAction::triggered, this, &MusicListInfoView::slotRemoveSongListActionClicked);
    connect(deleteLocalAction, &QAction::triggered, this, &MusicListInfoView::slotDeleteLocalActionClicked);

    mainMenu.exec(globalPos);
}

void MusicListInfoView::slotPlayListMenuClicked(QAction *action)
{
    qDebug() << action;
}

void MusicListInfoView::slotPlayActionClicked(bool checked)
{
    Q_UNUSED(checked)

    if (m_currMeta.hash == Player::instance()->activeMeta().hash) {
        Player::instance()->resume();
    } else {
        Player::instance()->playMeta(m_currMeta);
    }
}

void MusicListInfoView::slotPauseActionClicked(bool checked)
{
    Q_UNUSED(checked)

    Player::instance()->pause();
}

void MusicListInfoView::slotFileManagementShowClicked(bool checked)
{
    auto dirUrl = QUrl::fromLocalFile(m_currMeta.localPath);
    Dtk::Widget::DDesktopServices::showFileItem(dirUrl);
}

void MusicListInfoView::slotMusicInfoActionClicked(bool checked)
{
    Q_UNUSED(checked)

    InfoDialog infoDialog;
    infoDialog.updateInfo(m_currMeta);
    infoDialog.exec();
}

void MusicListInfoView::slotRemoveSongListActionClicked(bool checked)
{
    Q_UNUSED(checked)

//    MetaPtrList metalist;
//    for (auto index : selection->selectedRows()) {
//        auto meta = d->model->meta(index);
//        metalist << meta;
//    }
//    if (metalist.isEmpty())
//        return ;

//    Dtk::Widget::DDialog warnDlg(this);
//    warnDlg.setTextFormat(Qt::RichText);
//    warnDlg.addButton(tr("Cancel"), true, Dtk::Widget::DDialog::ButtonNormal);
//    int deleteFlag = warnDlg.addButton(tr("Remove"), false, Dtk::Widget::DDialog::ButtonWarning);

//    if (1 == metalist.length()) {
//        auto meta = metalist.first();
//        warnDlg.setMessage(QString(tr("Are you sure you want to remove %1?")).arg(meta->title));
//    } else {
//        warnDlg.setMessage(QString(tr("Are you sure you want to remove the selected %1 songs?").arg(metalist.length())));
//    }

//    warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
//    if (deleteFlag == warnDlg.exec()) {
//        d->removeSelection(selection);
//    }
}

void MusicListInfoView::slotDeleteLocalActionClicked(bool checked)
{
    Q_UNUSED(checked)

    QStringList metaList;
    metaList.append(m_currMeta.hash);


    Dtk::Widget::DDialog warnDlg(this);
    warnDlg.setTextFormat(Qt::RichText);
    warnDlg.addButton(tr("Cancel"), true, Dtk::Widget::DDialog::ButtonWarning);
    warnDlg.addButton(tr("Delete"), false, Dtk::Widget::DDialog::ButtonNormal);

    auto cover = QImage(QString(":/common/image/del_notify.svg"));
    MediaMeta meta = this->currentIndex().data(Qt::UserRole).value<MediaMeta>();
    //            QByteArray coverData = meta.getCoverData("12");
    //            if (coverData.length() > 0) {
    //                cover = QImage::fromData(coverData);
    //            }
    warnDlg.setMessage(QString(tr("Are you sure you want to delete %1?")).arg(meta.title));

    warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
    if (1 == warnDlg.exec()) {
        DataBaseService::getInstance()->removeSelectedSongs(m_hash, metaList, true);
    }
}

//void MusicListInfoView::onMusiclistChanged(PlaylistPtr playlist, const QString name)
//{
//    Q_D(MusicListInfoView);

//    if (playlist.isNull()) {
//        qWarning() << "can not change to emptry playlist";
//        return;
//    }

//    d->curName     = name;

//    d->model->removeRows(0, d->model->rowCount());
//    for (auto TypePtr : playlist->playMusicTypePtrList()) {
//        if (TypePtr->name == name) {
//            for (auto metaHash : TypePtr->playlistMeta.sortMetas) {
//                if (TypePtr->playlistMeta.metas.contains(metaHash))
//                    d->addMedia(TypePtr->playlistMeta.metas[metaHash]);
//            }
//        }
//    }

//    d->model->setPlaylist(playlist);
//    //updateScrollbar();
//}

void MusicListInfoView::keyPressEvent(QKeyEvent *event)
{
    switch (event->modifiers()) {
    case Qt::NoModifier:
        switch (event->key()) {
        case Qt::Key_Delete:
            QItemSelectionModel *selection = this->selectionModel();
//            removeSelection(selection);
            break;
        }
        break;
    case Qt::ShiftModifier:
        switch (event->key()) {
        case Qt::Key_Delete:
            break;
        }
        break;
    case Qt::ControlModifier:
        switch (event->key()) {
        case Qt::Key_I:
            QItemSelectionModel *selection = this->selectionModel();
            if (selection->selectedRows().length() <= 0) {
                return;
            }
            auto index = selection->selectedRows().first();
//            auto meta = m_model->meta(index);
//            Q_EMIT showInfoDialog(meta);
            break;
        }
        break;
    //    case Qt::ControlModifier:
    //        switch (event->key()) {
    //        case Qt::Key_K:
    //            QItemSelectionModel *selection = this->selectionModel();
    //            if (selection->selectedRows().length() > 0) {
    //                MetaPtrList metalist;
    //                for (auto index : selection->selectedRows()) {
    //                    auto meta = d->model->meta(index);
    //                    metalist << meta;
    //                }
    //                if (!metalist.isEmpty())
    //                    Q_EMIT addMetasFavourite(metalist);
    //            }
    //            break;
    //        }
    //        break;
    //    case Qt::ControlModifier | Qt::ShiftModifier:
    //        switch (event->key()) {
    //        case Qt::Key_K:
    //            QItemSelectionModel *selection = this->selectionModel();
    //            if (selection->selectedRows().length() > 0) {
    //                MetaPtrList metalist;
    //                for (auto index : selection->selectedRows()) {
    //                    auto meta = d->model->meta(index);
    //                    metalist << meta;
    //                }
    //                if (!metalist.isEmpty())
    //                    Q_EMIT removeMetasFavourite(metalist);
    //            }
    //            break;
    //        }
    //        break;
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

void MusicListInfoView::addMedia(MediaMeta meta)
{
    QStandardItem *newItem = new QStandardItem;
    QFileInfo coverInfo(Global::cacheDir() + "/images/" + meta.hash + ".jpg");
    if (coverInfo.exists()) {
        QIcon icon = QIcon(Global::cacheDir() + "/images/" + meta.hash + ".jpg");
        newItem->setIcon(icon);
    } else {
        newItem->setIcon(m_icon);
    }
    m_model->appendRow(newItem);

    auto row = m_model->rowCount() - 1;
    QModelIndex index = m_model->index(row, 0, QModelIndex());
    QVariant metaVar;
    metaVar.setValue(meta);
    m_model->setData(index, metaVar, Qt::UserRole);
}

void MusicListInfoView::onDoubleClicked(const QModelIndex &index)
{
    MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();

    Player::instance()->playMeta(meta);
    Player::instance()->setCurrentPlayListHash(m_hash, false);
}

void MusicListInfoView::dragEnterEvent(QDragEnterEvent *event)
{
    QListView::dragEnterEvent(event);
}

void MusicListInfoView::startDrag(Qt::DropActions supportedActions)
{

}

