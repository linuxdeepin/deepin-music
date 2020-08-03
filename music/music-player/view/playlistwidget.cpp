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
#include <DGuiApplicationHelper>

#include "../core/music.h"
#include "../core/playlist.h"
#include "../core/musicsettings.h"
#include "../core/util/inotifyfiles.h"
#include "../core/util/threadpool.h"
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
    InotifyFiles        inotifyFiles;

    PlayListWidget *q_ptr;
    Q_DECLARE_PUBLIC(PlayListWidget)
};


void PlayListWidgetPrivate::initData(PlaylistPtr playlist)
{
    Q_Q(PlayListWidget);

    if (playlist->id() != PlayMusicListID)
        return;

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
    q->connect(playListView, &PlayListView::resume,
    q, [ = ](const MetaPtr meta) {
        Q_EMIT q->resume(playListView->playlist(), meta);
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
    q, [ = ](const QString & preTitle, const QString & preArtist, const QString & preAlbum, const MetaPtr  meta) {
        Q_EMIT q->updateMetaCodec(preTitle, preArtist, preAlbum, meta);
    });
    q->connect(playListView, &PlayListView::addMetasFavourite,
    q, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT q->addMetasFavourite(metalist);
    });
    q->connect(playListView, &PlayListView::removeMetasFavourite,
    q, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT q->removeMetasFavourite(metalist);
    });

    q->connect(&inotifyFiles, &InotifyFiles::fileChanged,
    q, [ = ](const QStringList  & files) {
        auto allMetas = playListView->playlist()->allmusic();
        int allCount = allMetas.size();
        int missCount = 0;
        if (!allMetas.isEmpty()) {
            MetaPtrList  metalist;
            for (auto file : files) {
                for (int i = 0; i < allMetas.size(); i++) {
                    if (file == allMetas[i]->localPath) {
                        metalist.append(allMetas[i]);
                        allMetas.removeAt(i);
                        missCount++;
                        //break;
                    }
                }
            }

            if (allCount == missCount) {
                if (allCount == 1)
                    Q_EMIT q->fileRemoved(playListView->playlist(), metalist.at(0), 1);
                else {
                    for (MetaPtr meta : metalist) {
                        if (meta == playListView->activingMeta()) {
                            Q_EMIT q->fileRemoved(playListView->playlist(), metalist.at(0), 1);
                        }
                    }
                }
                Q_EMIT q->musiclistRemove(playListView->playlist(), playListView->playlist()->allmusic());
            } else if (missCount > 0) {
                /***************************************************************
                 * stop current music
                 * *************************************************************/
                Q_EMIT q->musiclistRemove(playListView->playlist(), metalist);
                /****************************************************************
                 * emit file not found
                 * 1 = Player::ResourceError
                 * ***************************************************************/
                for (MetaPtr meta : metalist) {
                    if (meta == playListView->activingMeta())
                        Q_EMIT q->fileRemoved(playListView->playlist(), metalist.at(0), 1);
                }
            }

            if (!metalist.isEmpty()) {
                playListView->playlist()->removeMusicList(metalist);
            }
        }
        Q_EMIT q->musicFileMiss();
    });
    q->connect(playListView, &PlayListView::hideEmptyHits, q, [ = ](bool a) {
        showEmptyHits(a);
    });
}

void PlayListWidgetPrivate::showEmptyHits(bool empty)
{
    auto playlist = playListView->playlist();
    if (playlist.isNull() || playlist->id() != SearchMusicListID) {
        emptyHits->setText(PlayListWidget::tr("No songs"));
    } else {
        emptyHits->setText(PlayListWidget::tr("No search results"));
    }
    playListView->setVisible(!empty);
    emptyHits->setVisible(empty);
}

PlayListWidget::PlayListWidget(QWidget *parent) :
    DWidget(parent), d_ptr(new PlayListWidgetPrivate(this))
{
    Q_D(PlayListWidget);

//    setFixedHeight(314);
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
    //d->titleLabel->setFixedHeight(36);
    auto titleFont = d->titleLabel->font();
    titleFont.setFamily("SourceHanSansSC");
    titleFont.setWeight(QFont::Medium);
    titleFont.setPixelSize(24);
    d->titleLabel->setFont(titleFont);
    d->titleLabel->setMargin(4);
    d->titleLabel->setText(tr("Play Queue"));
    d->titleLabel->setForegroundRole(DPalette::BrightText);

    d->infoLabel = new DLabel();
    //d->infoLabel->setFixedHeight(25);
    auto infoFont = d->infoLabel->font();
    infoFont.setFamily("SourceHanSansSC");
    infoFont.setWeight(QFont::Medium);
    infoFont.setPixelSize(14);
    d->infoLabel->setFont(infoFont);
    d->infoLabel->setMargin(4);
    auto infoLabelPalette = d->infoLabel->palette();
    QColor infoLabelColor = infoLabelPalette.color(DPalette::BrightText);
    infoLabelColor.setAlphaF(0.5);
    infoLabelPalette.setColor(DPalette::ButtonText, infoLabelColor);
    d->infoLabel->setPalette(infoLabelPalette);
    d->infoLabel->setForegroundRole(DPalette::ButtonText);

    d->btClearAll = new DPushButton;
    d->btClearAll->setIcon(QIcon(":/mpimage/light/normal/clear_list_normal.svg"));
    d->btClearAll->setFont(infoFont);
    auto playAllPalette = d->btClearAll->palette();
    playAllPalette.setColor(DPalette::ButtonText, Qt::white);
    playAllPalette.setColor(DPalette::Dark, QColor(Qt::darkGray));
    playAllPalette.setColor(DPalette::Light, QColor(Qt::darkGray));
    d->btClearAll->setPalette(playAllPalette);
    d->btClearAll->setObjectName("PlayListPlayAll");
    d->btClearAll->setText(tr("Empty"));
    d->btClearAll->setFocusPolicy(Qt::NoFocus);
    d->btClearAll->setFixedHeight(30);

    d->emptyHits = new DLabel(this);
    d->emptyHits->setObjectName("PlayListEmptyHits");
    d->emptyHits->hide();

    actionBarLayout->addWidget(d->titleLabel);
    actionBarLayout->addWidget(d->infoLabel);
    actionBarLayout->addWidget(d->btClearAll, 0, Qt::AlignLeft);
    actionBarLayout->addStretch();

    d->playListView = new PlayListView(false, true);
    d->playListView->hide();

    layout->addWidget(d->actionBar);
    layout->addWidget(d->playListView);
    layout->addWidget(d->emptyHits, 0, Qt::AlignCenter);

    d->initConntion();

    int themeType = DGuiApplicationHelper::instance()->themeType();
    slotTheme(themeType);

    d->inotifyFiles.start();
    ThreadPool::instance()->moveToNewThread(&d->inotifyFiles);
}

PlayListWidget::~PlayListWidget()
{
}

void PlayListWidget::updateInfo(PlaylistPtr playlist)
{
    Q_D(PlayListWidget);

    QString infoStr;
    int sortMetasSize = playlist->allmusic().size();
    //int sortMetasSize = d->playListView->rowCount();
    if (sortMetasSize == 0) {
        infoStr = tr("No songs");
    } else if (sortMetasSize == 1) {
        infoStr = tr("1 song");
    } else {
        infoStr = tr("%1 songs").arg(sortMetasSize);
    }
    d->infoLabel->setText(infoStr);

    if (playlist.isNull() || playlist->id() != PlayMusicListID || playlist->allmusic().isEmpty())
        return;

    d->inotifyFiles.clear();
    for (auto curMeta : playlist->allmusic()) {
        d->inotifyFiles.addPath(curMeta->localPath);
    }

}

void PlayListWidget::setCurPlaylist(PlaylistPtr playlist)
{
    Q_D(PlayListWidget);
    d->initData(playlist);
}

PlaylistPtr PlayListWidget::curPlaylist()
{
    Q_D(PlayListWidget);
    return d->playListView->playlist();
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
    //Q_D(PlayListWidget);
    DWidget::resizeEvent(event);
}

void PlayListWidget::onMusicPlayed(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(PlayListWidget);

    if (d->playListView->rowCount() == 0 || playlist != d->playListView->playlist() ||
            playlist->allmusic().size() != d->playListView->rowCount() ||
            playlist->first()->hash != d->playListView->firstHash())
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

    if (playlist.isNull())
        return;

    if (playlist != d->playListView->playlist() && d->playListView->rowCount() != playlist->allmusic().size()) {
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

    if (playlist->id() != PlayMusicListID) {
        return;
    }

    if (playlist != d->playListView->playlist() && d->playListView->rowCount() != playlist->allmusic().size()) {
        return;
    }
//    MetaPtrList curMetalist;
//    for (auto curMeta : metalist) {
//        if (!playlist->contains(curMeta))
//            curMetalist.append(curMeta);
//    }
//    if (curMetalist.isEmpty())
//        return;

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
    if (type == 0)
        type = DGuiApplicationHelper::instance()->themeType();
    if (type == 1) {
//        auto titleLabelPalette = d->infoLabel->palette();
//        titleLabelPalette.setColor(DPalette::ButtonText, ("#000000"));
//        d->titleLabel->setPalette(titleLabelPalette);
//        d->titleLabel->setForegroundRole(DPalette::ButtonText);

        auto infoLabelPalette = d->infoLabel->palette();
        QColor infoLabelColor = infoLabelPalette.color(DPalette::BrightText);
        infoLabelColor.setAlphaF(0.5);
        infoLabelPalette.setColor(DPalette::ButtonText, infoLabelColor);
        d->infoLabel->setPalette(infoLabelPalette);

        DPalette pl = d->btClearAll ->palette();
        pl.setColor(DPalette::ButtonText, QColor("#FFFFFF"));
        pl.setColor(DPalette::Light, QColor("#646464"));
        pl.setColor(DPalette::Dark, QColor("#5C5C5C"));
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.08);
        pl.setColor(DPalette::Shadow, sbcolor);
        d->btClearAll->setPalette(pl);
    } else {
//        auto titleLabelPalette = d->infoLabel->palette();
//        titleLabelPalette.setColor(DPalette::ButtonText, ("#FFFFFF"));
//        d->titleLabel->setPalette(titleLabelPalette);
//        d->titleLabel->setForegroundRole(DPalette::ButtonText);

        auto infoLabelPalette = d->infoLabel->palette();
        QColor infoLabelColor = infoLabelPalette.color(DPalette::BrightText);
        infoLabelColor.setAlphaF(0.5);
        infoLabelPalette.setColor(DPalette::ButtonText, infoLabelColor);
        d->infoLabel->setPalette(infoLabelPalette);

        DPalette pl = d->btClearAll->palette();
        pl.setColor(DPalette::ButtonText, QColor("#FFFFFF"));
        pl.setColor(DPalette::Light, QColor("#555454"));
        pl.setColor(DPalette::Dark, QColor("#414141"));
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.08);
        pl.setColor(DPalette::Shadow, sbcolor);
        d->btClearAll->setPalette(pl);
    }
    d->playListView->setThemeType(type);
}

void PlayListWidget::changePicture(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap)
{
    Q_D(PlayListWidget);
    d->playListView->setPlayPixmap(pixmap, sidebarPixmap, albumPixmap);
}

