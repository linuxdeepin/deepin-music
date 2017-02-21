/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musiclistview.h"

#include <QDebug>
#include <QMenu>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QStyleFactory>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QScrollBar>
#include <ddialog.h>

#include "../../core/metasearchservice.h"
#include "../helper/widgethellper.h"
#include "../helper/thememanager.h"

#include "musicitemdelegate.h"
#include "infodialog.h"

class MusicListViewPrivate
{
public:
    MusicListViewPrivate(MusicListView *parent): q_ptr(parent) {}

    void addMedia(const MetaPtr meta);
    void checkScrollbarSize();
    void removeSelection(QItemSelectionModel *selection);

    PlaylistPtr         currentPlaylist;
    QStandardItemModel  *model        = nullptr;
    MusicItemDelegate   *delegate     = nullptr;
    QScrollBar          *vscrollBar    = nullptr;

    MusicListView *q_ptr;
    Q_DECLARE_PUBLIC(MusicListView)
};

MusicListView::MusicListView(QWidget *parent)
    : QListView(parent), d_ptr(new MusicListViewPrivate(this))
{
    Q_D(MusicListView);

    setObjectName("MusicListView");

    d->model = new QStandardItemModel(0, 5, this);
    setModel(d->model);

    d->delegate = new MusicItemDelegate;
    setItemDelegate(d->delegate);

    setSelectionMode(QListView::ExtendedSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MusicListView::customContextMenuRequested,
            this, &MusicListView::requestCustomContextMenu);

    d->vscrollBar = new QScrollBar(this);
    d->vscrollBar->setObjectName("MusicListViewScrollBar");
    d->vscrollBar->setOrientation(Qt::Vertical);
    d->vscrollBar->raise();

    connect(d->vscrollBar, &QScrollBar::valueChanged,
    this, [ = ](int value) {
        verticalScrollBar()->setValue(value);
    });
    ThemeManager::instance()->regisetrWidget(this);
}

MusicListView::~MusicListView()
{

}

MetaPtr MusicListView::activingMeta() const
{
    Q_D(const MusicListView);

    if (d->currentPlaylist.isNull()) {
        return MetaPtr();
    }

    return d->currentPlaylist->playing();
}

QModelIndex MusicListView::findIndex(const MetaPtr meta)
{
    Q_ASSERT(!meta.isNull());
    Q_D(MusicListView);

    QModelIndex index;
    for (int i = 0; i < d->model->rowCount(); ++i) {
        index = d->model->index(i, 0);
        auto itemMeta = qvariant_cast<MetaPtr>(d->model->data(index));
        if (itemMeta.isNull()) {
            continue;
        }

        if (itemMeta->hash == meta->hash) {
            break;
        }
    }
    return index;
}

void MusicListView::onMusicListRemoved(const MetaPtrList metalist)
{
    Q_D(MusicListView);

    for (auto meta : metalist) {
        if (meta.isNull()) {
            continue;
        }

        for (int i = 0; i < d->model->rowCount(); ++i) {
            auto index = d->model->index(i, 0);
            auto itemMeta = qvariant_cast<MetaPtr>(d->model->data(index));
            if (itemMeta.isNull()) {
                qCritical() << "index" << index << i;
                continue;
            }

            if (meta->hash == itemMeta->hash) {
                d->model->removeRow(i);
            }
        }
    }
    d->checkScrollbarSize();
}

void MusicListView::onMusicError(const MetaPtr meta, int error)
{
    Q_ASSERT(!meta.isNull());
    Q_D(MusicListView);

    qDebug() << error;
//    QModelIndex index = findIndex(meta);

//    auto indexData = index.data().value<MetaPtr>();
//    indexData.invalid = (error != 0);
//    d->m_model->setData(index, QVariant::fromValue<MetaPtr>(indexData));

    update();
}

void MusicListView::onMusicListAdded(const MetaPtrList metalist)
{
    Q_D(MusicListView);
    for (auto meta : metalist) {
        d->addMedia(meta);
    }
    d->checkScrollbarSize();
}

void MusicListView::onLocate(const MetaPtr meta)
{
    QModelIndex index = findIndex(meta);
    if (!index.isValid()) {
        return;
    }

    clearSelection();

    auto viewRect = QRect(QPoint(0, 0), size());
    if (!viewRect.intersects(visualRect(index))) {
        scrollTo(index, MusicListView::PositionAtCenter);
    }
    setCurrentIndex(index);
}

void MusicListView::onMusiclistChanged(PlaylistPtr playlist)
{
    Q_D(MusicListView);

    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }

    d->model->removeRows(0, d->model->rowCount());
    for (auto meta : playlist->allmusic()) {
        d->addMedia(meta);
    }

    d->currentPlaylist = playlist;
    d->checkScrollbarSize();
}

void MusicListView::wheelEvent(QWheelEvent *event)
{
    Q_D(MusicListView);
    QListView::wheelEvent(event);
    d->vscrollBar->setSliderPosition(verticalScrollBar()->sliderPosition());
}

void MusicListView::resizeEvent(QResizeEvent *event)
{
    Q_D(MusicListView);
    QListView::resizeEvent(event);
    d->checkScrollbarSize();
}

void MusicListView::keyPressEvent(QKeyEvent *event)
{
    Q_D(MusicListView);
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
    default: break;
    }

    QAbstractItemView::keyPressEvent(event);
}

void MusicListViewPrivate::addMedia(const MetaPtr meta)
{
    QStandardItem *newItem = new QStandardItem;
    model->appendRow(newItem);

    auto row = model->rowCount() - 1;
    QModelIndex index = model->index(row, 0, QModelIndex());
    model->setData(index, QVariant::fromValue<MetaPtr>(meta));
}

void MusicListViewPrivate::checkScrollbarSize()
{
    Q_Q(MusicListView);

    auto itemCount = q->model()->rowCount();
    auto size = q->size();
    auto scrollBarWidth = 8;
    vscrollBar->resize(scrollBarWidth, size.height() - 2);
    vscrollBar->move(size.width() - scrollBarWidth - 2, 0);
    vscrollBar->setSingleStep(1);
    vscrollBar->setPageStep(size.height() / 36);
    vscrollBar->setMinimum(0);

    if (itemCount > size.height() / 36) {
        vscrollBar->show();
        vscrollBar->setMaximum(itemCount - size.height() / 36);
    } else {
        vscrollBar->hide();
        vscrollBar->setMaximum(0);
    }
}

void MusicListViewPrivate::removeSelection(QItemSelectionModel *selection)
{
    Q_ASSERT(selection != nullptr);
    Q_Q(MusicListView);

    MetaPtrList metalist;
    for (auto index : selection->selectedRows()) {
        auto meta = qvariant_cast<MetaPtr>(model->data(index));
        metalist << meta;
    }
    emit q->removeMusicList(metalist);
}

void MusicListView::showContextMenu(const QPoint &pos,
                                    PlaylistPtr selectedPlaylist,
                                    PlaylistPtr favPlaylist,
                                    QList<PlaylistPtr> newPlaylists)
{
    Q_D(MusicListView);
    QItemSelectionModel *selection = this->selectionModel();

    if (selection->selectedRows().length() <= 0) {
        return;
    }

    QPoint globalPos = this->mapToGlobal(pos);

    QMenu playlistMenu;
    playlistMenu.setStyle(QStyleFactory::create("dlight"));

    auto newvar = QVariant::fromValue(PlaylistPtr());

    auto createPlaylist = playlistMenu.addAction(tr("New playlist"));
    createPlaylist->setData(newvar);
    createPlaylist->setIcon(QIcon(":/light/image/plus.png"));

    playlistMenu.addSeparator();

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

    connect(&playlistMenu, &QMenu::triggered, this, [ = ](QAction * action) {
        auto playlist = action->data().value<PlaylistPtr >();
        MetaPtrList metalist;
        for (auto &index : selection->selectedRows()) {
            auto meta = qvariant_cast<MetaPtr>(d->model->data(index));
            if (!meta.isNull()) {
                metalist << meta;
            }
        }
        emit addToPlaylist(playlist, metalist);
    });

    bool singleSelect = (1 == selection->selectedRows().length());

    QMenu myMenu;
    myMenu.setStyle(QStyleFactory::create("dlight"));

    QAction *playAction = nullptr;
    if (singleSelect) {
        playAction = myMenu.addAction(tr("Play"));
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

    QMenu textCodecMenu;
    textCodecMenu.setStyle(QStyleFactory::create("dlight"));

    if (singleSelect) {
        auto index = selection->selectedRows().first();
        auto meta = qvariant_cast<MetaPtr>(d->model->data(index));
        QList<QByteArray> codecList = DMusic::detectMetaEncodings(meta);

//        auto defaultCodec = QTextCodec::codecForLocale()->name();
//        qDebug() << defaultCodec << codecList.length();

//        codecList.removeAll(defaultCodec);
//        auto act = textCodecMenu.addAction(defaultCodec);
//        act->setData(QVariant::fromValue(defaultCodec));
//        textCodecMenu.addSeparator();

//        qDebug() << codecList;
//        codecList << "GB18030" << "Big5"
//                  << "IBM420_ltr" << "UTF-16BE"
//                  << "UTF-16LE" << "EUC-JP"
//                  << "EUC-KR" << "ISO-8859-1"
//                  << "windows-1256" << "windows-1251"
//                  << "Shift_JIS" << "GB18030" << "EUC-JP" << "EUC-KR" << "Big5";

        for (auto codec : codecList) {
            auto act = textCodecMenu.addAction(codec);
            act->setData(QVariant::fromValue(codec));
        }

        if (codecList.length() > 1) {
            myMenu.addSeparator();
            myMenu.addAction(tr("Text Encoding"))->setMenu(&textCodecMenu);
        }

        myMenu.addSeparator();
        songAction = myMenu.addAction(tr("Song info"));

        connect(&textCodecMenu, &QMenu::triggered, this, [ = ](QAction * action) {
//            auto codec = action->data().toByteArray();
//            MetaPtr updateMeta = meta;
//            MusicMetaName::updateCodec(updateMeta, codec);
//            emit updateMetaCodec(updateMeta);
//            qDebug() << codec;
//            d->model->setData(index, QVariant::fromValue<MediaMeta>(updateMeta));
//            item->setData(QVariant::fromValue<MediaMeta>(updateMeta));
//            this->update();
        });
    }

    if (playAction) {
        connect(playAction, &QAction::triggered, this, [ = ](bool) {
            auto index = selection->selectedRows().first();
            auto meta = qvariant_cast<MetaPtr>(d->model->data(index));
            emit playMedia(meta);
        });
    }

    if (displayAction) {
        connect(displayAction, &QAction::triggered, this, [ = ](bool) {
            auto index = selection->selectedRows().first();
            auto meta = qvariant_cast<MetaPtr>(d->model->data(index));

            auto dirUrl = QUrl::fromLocalFile(QFileInfo(meta->localPath).absoluteDir().absolutePath());
            QFileInfo ddefilemanger("/usr/bin/dde-file-manager");
            if (ddefilemanger.exists()) {
                auto dirFile = QUrl::fromLocalFile(QFileInfo(meta->localPath).absoluteFilePath());
                auto url = QString("%1?selectUrl=%2").arg(dirUrl.toString()).arg(dirFile.toString());
                QProcess::startDetached("dde-file-manager" , QStringList() << url);
            } else {
                QProcess::startDetached("gvfs-open" , QStringList() << dirUrl.toString());
            }
        });
    }

    if (removeAction) {
        connect(removeAction, &QAction::triggered, this, [ = ](bool) {
            \
            d->removeSelection(selection);
        });
    }

    if (deleteAction) {
        connect(deleteAction, &QAction::triggered, this, [ = ](bool) {
            bool containsCue = false;
            MetaPtrList metalist;
            for (auto index : selection->selectedRows()) {
                auto meta = qvariant_cast<MetaPtr>(d->model->data(index));
                if (!meta->cuePath.isEmpty()) {
                    containsCue = true;
                }
                metalist << meta;
            }

            Dtk::Widget::DDialog warnDlg(this);
            warnDlg.setStyle(QStyleFactory::create("dlight"));
            warnDlg.setTextFormat(Qt::RichText);
            warnDlg.addButtons(QStringList() << tr("Cancel") << tr("Delete"));

            auto cover = QImage(QString(":/common/image/del_notify.png"));
            if (1 == metalist.length()) {
                auto meta = metalist.first();
                auto coverData = MetaSearchService::coverData(meta);
                if (coverData.length() > 0) {
                    cover = QImage::fromData(coverData);
                }
                warnDlg.setMessage(
                    QString(tr("Are you sure to delete %1?")).arg(meta->title));
            } else {
                warnDlg.setMessage(
                    QString(tr("Are you sure to delete the selected %1 songs?")).arg(metalist.length()));
            }

            if (containsCue) {
                warnDlg.setTitle(tr("Are you sure to delete the selected %1 song files?").arg(metalist.length()));
                warnDlg.setMessage(tr("Other song of the same file will be deleted too."));
            }
            auto coverPixmap =  QPixmap::fromImage(WidgetHelper::cropRect(cover, QSize(64, 64)));

            warnDlg.setIcon(QIcon(coverPixmap));
            if (0 == warnDlg.exec()) {
                return;
            }
            emit deleteMusicList(metalist);
        });
    }

    if (songAction) {
        connect(songAction, &QAction::triggered, this, [ = ](bool) {
            auto index = selection->selectedRows().first();
            auto meta = qvariant_cast<MetaPtr>(d->model->data(index));
            QPixmap coverPixmap;
            auto coverData = MetaSearchService::coverData(meta);
            if (coverData.length() > 0) {
                QImage cover;
                cover = QImage::fromData(coverData);
                coverPixmap = QPixmap::fromImage(WidgetHelper::cropRect(cover, QSize(140, 140)));
            }

            InfoDialog dlg(meta, this);
            dlg.setStyle(QStyleFactory::create("dlight"));
            // FIXME: qss only work after show
            dlg.show();
            dlg.setCoverImage(coverPixmap);
            dlg.initUI(meta);
            dlg.updateLabelSize();
            dlg.exec();
        });
    }

    myMenu.exec(globalPos);
}

