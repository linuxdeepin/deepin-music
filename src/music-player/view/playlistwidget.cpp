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

#include "playlistwidget.h"

#include <QDebug>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMimeData>
#include <QResizeEvent>
#include <QStandardItemModel>

#include <DPushButton>
#include <DComboBox>
#include <DLabel>
#include <DPalette>

#include "../core/music.h"
#include "../core/playlist.h"
#include "../core/musicsettings.h"
#include "widget/playlistview.h"
#include "widget/ddropdown.h"

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class PlayListWidgetPrivate
{
public:
    PlayListWidgetPrivate(PlayListWidget *parent) : q_ptr(parent) {}

    void initData(PlaylistPtr playlist);
    void initConntion();
    void showEmptyHits(bool empty);

    DLabel              *titleLabel     = nullptr;
    DLabel              *infoLabel      = nullptr;
    DLabel              *emptyHits      = nullptr;
    DWidget             *actionBar      = nullptr;
    DPushButton         *btClearAll     = nullptr;
    PlayListView        *playListView   = nullptr;
    QAction             *customAction   = nullptr;

    PlayListWidget *q_ptr;
    Q_DECLARE_PUBLIC(PlayListWidget)
};


void PlayListWidgetPrivate::initData(PlaylistPtr playlist)
{
    Q_Q(PlayListWidget);

    q->updateInfo(playlist);

    playListView->onMusiclistChanged(playlist);

    showEmptyHits(playListView->model()->rowCount() == 0);
}

void PlayListWidgetPrivate::initConntion()
{
    Q_Q(PlayListWidget);

    q->connect(btClearAll, &DPushButton::clicked,
    q, [ = ](bool) {
        if (playListView->playlist()) {
            Q_EMIT q->musiclistRemove(playListView->playlist(), playListView->playlist()->allmusic());
        }
    });

    q->connect(playListView, &PlayListView::requestCustomContextMenu,
    q, [ = ](const QPoint & pos) {
        Q_EMIT q->requestCustomContextMenu(pos, 0);
    });
    q->connect(playListView, &PlayListView::removeMusicList,
    q, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT q->musiclistRemove(playListView->playlist(), metalist);
        q->updateInfo(playListView->playlist());
    });
    q->connect(playListView, &PlayListView::deleteMusicList,
    q, [ = ](const MetaPtrList & metalist) {
        Q_EMIT q->musiclistDelete(playListView->playlist(), metalist);
        q->updateInfo(playListView->playlist());
    });
    q->connect(playListView, &PlayListView::addToPlaylist,
    q, [ = ](PlaylistPtr playlist, const MetaPtrList  metalist) {
        Q_EMIT q->addToPlaylist(playlist, metalist);
        q->updateInfo(playListView->playlist());
    });
    q->connect(playListView, &PlayListView::playMedia,
    q, [ = ](const MetaPtr meta) {
        Q_EMIT q->playMedia(playListView->playlist(), meta);
    });
    q->connect(playListView, &PlayListView::pause,
    q, [ = ](const MetaPtr meta) {
        Q_EMIT q->pause(playListView->playlist(), meta);
    });
    q->connect(playListView, &PlayListView::showInfoDialog,
    q, [ = ](const MetaPtr meta) {
        Q_EMIT q->showInfoDialog(meta);
    });
    q->connect(playListView, &PlayListView::updateMetaCodec,
    q, [ = ](const MetaPtr  meta) {
        Q_EMIT q->updateMetaCodec(meta);
    });
}

void PlayListWidgetPrivate::showEmptyHits(bool empty)
{
    auto playlist = playListView->playlist();
    if (playlist.isNull() || playlist->id() != SearchMusicListID) {
        emptyHits->setText(PlayListWidget::tr("No Music"));
    } else {
        emptyHits->setText(PlayListWidget::tr("No result found"));
    }
    playListView->setVisible(!empty);
    emptyHits->setVisible(empty);
}

PlayListWidget::PlayListWidget(QWidget *parent) :
    DBlurEffectWidget(parent), d_ptr(new PlayListWidgetPrivate(this))
{
    Q_D(PlayListWidget);

    setBlurRectXRadius(18);
    setBlurRectYRadius(18);
    setRadius(30);

    //setAutoFillBackground(true);

    setMode(DBlurEffectWidget::GaussianBlur);
    setBlurEnabled(true);
    //setBlendMode(DBlurEffectWidget::BehindWindowBlend);
    setMaskColor(QColor(200, 200, 200, 170));
    //setMaskAlpha(140);

    setObjectName("PlayListWidget");
    setAcceptDrops(true);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 30, 0, 0);
    layout->setSpacing(0);

    d->actionBar = new DWidget;
    d->actionBar->setFixedWidth(200);
    d->actionBar->setObjectName("PlayListActionBar");

    auto actionBarLayout = new QVBoxLayout(d->actionBar);
    actionBarLayout->setContentsMargins(36, 0, 8, 0);
    actionBarLayout->setSpacing(0);

    d->titleLabel = new DLabel();
    d->titleLabel->setFixedHeight(36);
    auto titleFont = d->titleLabel->font();
    titleFont.setFamily("SourceHanSansSC-Medium");
    titleFont.setPixelSize(24);
    d->titleLabel->setFont(titleFont);
    d->titleLabel->setMargin(4);
    d->titleLabel->setText(tr("Play List"));

    d->infoLabel = new DLabel();
    d->infoLabel->setFixedHeight(25);
    auto infoFont = d->infoLabel->font();
    infoFont.setFamily("SourceHanSansSC-Medium");
    infoFont.setPixelSize(14);
    d->infoLabel->setFont(infoFont);
    d->infoLabel->setMargin(4);
    auto infoLabelPalette = d->infoLabel->palette();
    infoLabelPalette.setColor(DPalette::ButtonText, QColor("#777777"));
    d->infoLabel->setPalette(infoLabelPalette);

    d->btClearAll = new DPushButton;
    d->btClearAll->setIcon(QIcon(":/mpimage/light/normal/clear_list_normal.svg"));
    d->btClearAll->setFont(infoFont);
    auto playAllPalette = d->btClearAll->palette();
    playAllPalette.setColor(DPalette::ButtonText, Qt::white);
    playAllPalette.setColor(DPalette::Dark, QColor(Qt::darkGray));
    playAllPalette.setColor(DPalette::Light, QColor(Qt::darkGray));
    d->btClearAll->setPalette(playAllPalette);
    d->btClearAll->setObjectName("PlayListPlayAll");
    d->btClearAll->setText(tr("Clear List"));
    d->btClearAll->setFocusPolicy(Qt::NoFocus);
    d->btClearAll->setFixedHeight(30);

    d->emptyHits = new DLabel();
    d->emptyHits->setObjectName("PlayListEmptyHits");
    d->emptyHits->hide();

    actionBarLayout->addWidget(d->titleLabel);
    actionBarLayout->addWidget(d->infoLabel);
    actionBarLayout->addWidget(d->btClearAll, 0, Qt::AlignLeft);
    actionBarLayout->addStretch();

    d->playListView = new PlayListView;
    d->playListView->hide();

    layout->addWidget(d->actionBar);
    layout->addWidget(d->playListView);
    layout->addWidget(d->emptyHits);

    d->initConntion();

    bool themeFlag = false;
    int themeType = MusicSettings::value("base.play.theme").toInt(&themeFlag);
    if (!themeFlag)
        themeType = 1;
    slotTheme(themeType);
}

PlayListWidget::~PlayListWidget()
{
}

void PlayListWidget::updateInfo(PlaylistPtr playlist)
{
    Q_D(PlayListWidget);
    QString infoStr;
    int sortMetasSize = playlist->allmusic().size();
    if (sortMetasSize == 0) {
        infoStr = tr("No songs");
    } else if (sortMetasSize == 1) {
        infoStr = tr("1 song");
    } else {
        infoStr = tr("%1 songs").arg(sortMetasSize);
    }
    d->infoLabel->setText(infoStr);
}

void PlayListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    DWidget::dragEnterEvent(event);
    if (event->mimeData()->hasFormat("text/uri-list")) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return;
    }
}

void PlayListWidget::dropEvent(QDropEvent *event)
{
    DWidget::dropEvent(event);
    Q_D(PlayListWidget);

    if (!event->mimeData()->hasFormat("text/uri-list")) {
        return;
    }

    auto urls = event->mimeData()->urls();
    QStringList localpaths;
    for (auto &url : urls) {
        localpaths << url.toLocalFile();
    }

    if (!localpaths.isEmpty() && !d->playListView->playlist().isNull()) {
        Q_EMIT importSelectFiles(d->playListView->playlist(), localpaths);
    }
}

void PlayListWidget::resizeEvent(QResizeEvent *event)
{
    Q_D(PlayListWidget);
    DWidget::resizeEvent(event);
}

void PlayListWidget::onMusicPlayed(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(PlayListWidget);

//    if (playlist != d->playListView->playlist()) {
//        d->initData(playlist);
//    }
    d->initData(playlist);

    if (playlist != d->playListView->playlist() || meta.isNull()) {
        return;
    }

    QModelIndex index = d->playListView->findIndex(meta);
    if (!index.isValid()) {
        return;
    }

    auto selectedIndexes = d->playListView->selectionModel()->selectedIndexes();
    if (selectedIndexes.size() > 1) {
        d->playListView->update();
        return;
    }

    d->playListView->clearSelection();
    d->playListView->setCurrentIndex(index);
    d->playListView->scrollTo(index);
    d->playListView->update();
}

void PlayListWidget::onMusicPause(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(PlayListWidget);
    if (playlist != d->playListView->playlist() || meta.isNull()) {
        return;
    }
    d->playListView->update();
}

void PlayListWidget::onMusicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_D(PlayListWidget);

    if (playlist != d->playListView->playlist()) {
        return;
    }

    d->playListView->onMusicListRemoved(metalist);
    d->showEmptyHits(d->playListView->model()->rowCount() == 0);
    updateInfo(playlist);
}

void PlayListWidget::onMusicError(PlaylistPtr playlist, const MetaPtr meta, int error)
{
    Q_D(PlayListWidget);
    Q_UNUSED(playlist);
    d->playListView->onMusicError(meta, error);
}

void PlayListWidget::onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_D(PlayListWidget);

    if (playlist != d->playListView->playlist()) {
        return;
    }

    d->playListView->onMusicListAdded(metalist);
    d->showEmptyHits(metalist.length() == 0);
    updateInfo(playlist);
}

void PlayListWidget::onLocate(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(PlayListWidget);
    if (d->playListView->playlist() != playlist) {
        d->initData(playlist);
    }
    d->playListView->onLocate(meta);
}

void PlayListWidget::onMusiclistChanged(PlaylistPtr playlist)
{
    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }

    Q_D(PlayListWidget);

    d->initData(playlist);
}

void PlayListWidget::onCustomContextMenuRequest(const QPoint &pos,
                                                PlaylistPtr selectedlist,
                                                PlaylistPtr favlist,
                                                QList<PlaylistPtr > newlists)
{
    Q_D(PlayListWidget);
    d->playListView->showContextMenu(pos, selectedlist, favlist, newlists);
}

void PlayListWidget::slotTheme(int type)
{
    Q_D(PlayListWidget);
    if (type == 1) {
        QColor maskColor(200, 200, 200, 170);
        setMaskColor(maskColor);
    } else {
        QColor maskColor(56, 56, 56, 170);
        setMaskColor(maskColor);
    }
    d->playListView->setThemeType(type);
}

void PlayListWidget::changePicture(QPixmap pixmap)
{
    Q_D(PlayListWidget);
    d->playListView->setPlayPixmap(pixmap);
}

