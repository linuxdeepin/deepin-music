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
#include "commonservice.h"

DWIDGET_USE_NAMESPACE

MusicListInfoView::MusicListInfoView(const QString &hash, QWidget *parent)
    : DListView(parent)
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
    connect(DataBaseService::getInstance(), &DataBaseService::sigRmvSong,
            this, &MusicListInfoView::slotRemoveSingleSong);
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
        pact->setData(QVariant(pd.uuid)); //to know which custom view to reach
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

    QString songlistHash = action->data().value<QString>();

    QItemSelectionModel *selection = selectionModel();
    QList<MediaMeta> metaList;
    for (int i = 0; i < selection->selectedRows().size(); i++) {
        QModelIndex curIndex = selection->selectedRows().at(i);
        MediaMeta meta = curIndex.data(Qt::UserRole).value<MediaMeta>();
        metaList.append(meta);
    }

    if (songlistHash == "song list") {
        emit CommonService::getInstance()->addNewSongList();

        if (metaList.size() > 0) {
            QString songlistUuid = DataBaseService::getInstance()->getCustomSongList().last().uuid;
            DataBaseService::getInstance()->addMetaToPlaylist(songlistUuid, metaList);
        }
    } else {
        DataBaseService::getInstance()->addMetaToPlaylist(songlistHash, metaList);
    }
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
    QModelIndexList modellist = selectionModel()->selectedRows();
    if (modellist.size() == 0)
        return;

    QStringList metaList;
    for (QModelIndex mindex : modellist) {
        MediaMeta imt = mindex.data(Qt::UserRole).value<MediaMeta>();
        metaList << imt.hash;
    }

    Dtk::Widget::DDialog warnDlg(this);
    warnDlg.setTextFormat(Qt::RichText);
    warnDlg.addButton(tr("Cancel"), true, Dtk::Widget::DDialog::ButtonNormal);
    int deleteFlag = warnDlg.addButton(tr("Remove"), false, Dtk::Widget::DDialog::ButtonWarning);

    MediaMeta meta = modellist.first().data(Qt::UserRole).value<MediaMeta>();
    if (1 == metaList.length()) {
        warnDlg.setMessage(QString(tr("Are you sure you want to remove %1?")).arg(meta.title));
    } else {
        warnDlg.setMessage(QString(tr("Are you sure you want to remove the selected %1 songs?").arg(metaList.size())));
    }

    warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
    if (deleteFlag == warnDlg.exec()) {
        DataBaseService::getInstance()->removeSelectedSongs("all", metaList, false);
        // 更新player中缓存的歌曲信息，如果存在正在播放的歌曲，停止播放
        Player::instance()->playRmvMeta(metaList);
    }
}

void MusicListInfoView::slotDeleteLocalActionClicked(bool checked)
{
    Q_UNUSED(checked)

    QList<MediaMeta> metas;
    QModelIndexList mindexlist =  this->selectedIndexes();
    QStringList strlist;
    for (QModelIndex mindex : mindexlist) {
        MediaMeta imt = mindex.data(Qt::UserRole).value<MediaMeta>();
        metas.append(imt);
        strlist << imt.hash;
    }

    Dtk::Widget::DDialog warnDlg(this);
    warnDlg.setTextFormat(Qt::RichText);
    warnDlg.addButton(tr("Cancel"), true, Dtk::Widget::DDialog::ButtonNormal);
    int deleteFlag = warnDlg.addButton(tr("Delete"), false, Dtk::Widget::DDialog::ButtonWarning);

    auto cover = QImage(QString(":/common/image/del_notify.svg"));
    if (1 == metas.length()) {
        auto meta = metas.first();
        warnDlg.setMessage(QString(tr("Are you sure you want to delete %1?")).arg(meta.title));
    } else {
        //                warnDlg.setTitle(QString(tr("Are you sure you want to delete the selected %1 songs?")).arg(metalist.length()));
        DLabel *t_titleLabel = new DLabel(this);
        t_titleLabel->setForegroundRole(DPalette::TextTitle);
        DLabel *t_infoLabel = new DLabel(this);
        t_infoLabel->setForegroundRole(DPalette::TextTips);
        t_titleLabel->setText(tr("Are you sure you want to delete the selected %1 songs?").arg(metas.length()));
        t_infoLabel->setText(tr("The song files contained will also be deleted"));
        warnDlg.addContent(t_titleLabel, Qt::AlignHCenter);
        warnDlg.addContent(t_infoLabel, Qt::AlignHCenter);
        warnDlg.addSpacing(20);
    }

    warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
    if (deleteFlag == warnDlg.exec()) {
        DataBaseService::getInstance()->removeSelectedSongs("all", strlist, true);
        Player::instance()->playRmvMeta(strlist);
    }
}

void MusicListInfoView::slotRemoveSingleSong(const QString &listHash, const QString &musicHash)
{
    if (listHash != "all") {
        return;
    }
    qDebug() << "---hash = " << musicHash;
    int row =  m_model->rowCount();
    for (int i = 0; i < row; i++) {
        QModelIndex mindex = m_model->index(i, 0, QModelIndex());
        MediaMeta meta = mindex.data(Qt::UserRole).value<MediaMeta>();
        if (meta.hash == musicHash) {
            this->removeItem(i);
            m_model->removeRow(row);
            this->viewport()->update();
            break;
        }
    }
}

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
    Player::instance()->clearPlayList();
    Player::instance()->setPlayList(getMusicListData());
    Player::instance()->setCurrentPlayListHash(m_hash, false);
    emit Player::instance()->signalPlayListChanged();

    MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();
    Player::instance()->playMeta(meta);
}

void MusicListInfoView::dragEnterEvent(QDragEnterEvent *event)
{
    QListView::dragEnterEvent(event);
}

void MusicListInfoView::startDrag(Qt::DropActions supportedActions)
{

}

