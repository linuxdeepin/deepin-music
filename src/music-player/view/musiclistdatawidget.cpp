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

#include "musiclistdatawidget.h"

#include <QDebug>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMimeData>
#include <QResizeEvent>
#include <QStandardItemModel>

#include <DLabel>
#include <DPushButton>
#include <DComboBox>
#include <DLabel>

#include "util/pinyinsearch.h"

#include "widget/playlistview.h"
#include "widget/musiclistdataview.h"
#include "widget/ddropdown.h"
#include "widget/musicimagebutton.h"

DWIDGET_USE_NAMESPACE

class MusicListDataWidgetPrivate
{
public:
    MusicListDataWidgetPrivate(MusicListDataWidget *parent) : q_ptr(parent) {}

    void initData(PlaylistPtr playlist);
    void initConntion();
    void showEmptyHits();

    DLabel              *emptyHits      = nullptr;
    DWidget             *actionBar      = nullptr;
    DLabel              *titleLabel     = nullptr;
    DDropdown           *albumDropdown  = nullptr;
    DDropdown           *artistDropdown = nullptr;
    DDropdown           *musicDropdown  = nullptr;
    DPushButton         *btPlayAll      = nullptr;
    DLabel              *infoLabel      = nullptr;
    MusicImageButton    *btIconMode     = nullptr;
    MusicImageButton    *btlistMode     = nullptr;
    MusicListDataView   *albumListView  = nullptr;
    MusicListDataView   *artistListView = nullptr;
    PlayListView        *musicListView  = nullptr;
    QAction             *customAction   = nullptr;
    PlaylistPtr         curPlaylist     = nullptr;

    MusicListDataWidget *q_ptr;
    Q_DECLARE_PUBLIC(MusicListDataWidget)
};

void MusicListDataWidgetPrivate::initData(PlaylistPtr playlist)
{
    Q_Q(MusicListDataWidget);

    curPlaylist = playlist;
    QString searchStr = playlist->searchStr();

    bool chineseFlag = false;
    for (auto ch : searchStr) {
        if (DMusic::PinyinSearch::isChinese(ch)) {
            chineseFlag = true;
            break;
        }
    }

    QFontMetrics titleFm(titleLabel->font());
    auto text = titleFm.elidedText(playlist->displayName(), Qt::ElideRight, 300);
    titleLabel->setText(text);
    titleLabel->setToolTip(playlist->displayName());
    DDropdown *t_curDropdown = nullptr;
    if (playlist->id() == AlbumMusicListID) {
        PlayMusicTypePtrList playMusicTypePtrList = playlist->playMusicTypePtrList();
        int musicCount = 0;
        int musicListCount = 0;
        for (auto action : playMusicTypePtrList) {
            if (searchStr.isEmpty()) {
                musicCount += action->playlistMeta.sortMetas.size();
                musicListCount++;
            } else {
                if (chineseFlag) {
                    if (action->name.contains(searchStr, Qt::CaseInsensitive)) {
                        musicCount += action->playlistMeta.sortMetas.size();
                        musicListCount++;
                    }
                } else {
                    if (searchStr.size() == 1) {
                        auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(action->name);
                        if (!curTextList.isEmpty() && curTextList.first().contains(searchStr, Qt::CaseInsensitive)) {
                            musicCount += action->playlistMeta.sortMetas.size();
                            musicListCount++;
                        }
                    } else {
                        auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(action->name);
                        if (!curTextList.isEmpty() && curTextList.join("").contains(searchStr, Qt::CaseInsensitive)) {
                            musicCount += action->playlistMeta.sortMetas.size();
                            musicListCount++;
                        }
                    }
                }
            }
        }
        QString infoStr;
        if (musicCount == 0) {
            infoStr = MusicListDataWidget::tr("   No songs");
        } else if (musicCount == 1) {
            infoStr = MusicListDataWidget::tr("   1 album-1 song");
        } else {
            infoStr = MusicListDataWidget::tr("   %1 album-%2 songs").arg(musicListCount).arg(musicCount);
            if (playMusicTypePtrList.size() == 1) {
                infoStr = MusicListDataWidget::tr("   %1 album-%2 songs").arg(musicListCount).arg(musicCount);
            } else {
                infoStr = MusicListDataWidget::tr("   %1 albums-%2 songs").arg(musicListCount).arg(musicCount);
            }
        }
        infoLabel->setText(infoStr);

        //update dropdown
        albumDropdown->show();
        artistDropdown->hide();
        musicDropdown->hide();

        albumListView->show();
        artistListView->hide();
        musicListView->hide();

        t_curDropdown = albumDropdown;

        if (albumListView->viewMode() == QListView::IconMode) {
            btIconMode->setChecked(true);
            btlistMode->setChecked(false);
        } else {
            btIconMode->setChecked(false);
            btlistMode->setChecked(true);
        }

        albumListView->onMusiclistChanged(playlist);
    } else if (playlist->id() == ArtistMusicListID) {
        PlayMusicTypePtrList playMusicTypePtrList = playlist->playMusicTypePtrList();
        int musicCount = 0;
        int musicListCount = 0;
        for (auto action : playMusicTypePtrList) {
            if (searchStr.isEmpty()) {
                musicCount += action->playlistMeta.sortMetas.size();
                musicListCount++;
            } else {
                if (chineseFlag) {
                    if (action->name.contains(searchStr, Qt::CaseInsensitive)) {
                        musicCount += action->playlistMeta.sortMetas.size();
                        musicListCount++;
                    }
                } else {
                    if (searchStr.size() == 1) {
                        auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(action->name);
                        if (!curTextList.isEmpty() && curTextList.first().contains(searchStr, Qt::CaseInsensitive)) {
                            musicCount += action->playlistMeta.sortMetas.size();
                            musicListCount++;
                        }
                    } else {
                        auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(action->name);
                        if (!curTextList.isEmpty() && curTextList.join("").contains(searchStr, Qt::CaseInsensitive)) {
                            musicCount += action->playlistMeta.sortMetas.size();
                            musicListCount++;
                        }
                    }
                }
            }
        }
        QString infoStr;
        if (musicCount == 0) {
            infoStr = MusicListDataWidget::tr("   No songs");
        } else if (musicCount == 1) {
            infoStr = MusicListDataWidget::tr("   1 artist-1 song");
        } else {
            if (playMusicTypePtrList.size() == 1) {
                infoStr = MusicListDataWidget::tr("   %1 artist-%2 songs").arg(musicListCount).arg(musicCount);
            } else {
                infoStr = MusicListDataWidget::tr("   %1 artists-%2 songs").arg(musicListCount).arg(musicCount);
            }
        }
        infoLabel->setText(infoStr);

        //update dropdown
        albumDropdown->hide();
        artistDropdown->show();
        musicDropdown->hide();

        albumListView->hide();
        artistListView->show();
        musicListView->hide();

        t_curDropdown = artistDropdown;

        if (artistListView->viewMode() == QListView::IconMode) {
            btIconMode->setChecked(true);
            btlistMode->setChecked(false);
        } else {
            btIconMode->setChecked(false);
            btlistMode->setChecked(true);
        }
        artistListView->onMusiclistChanged(playlist);
    } else {
        QString infoStr;
        int musicCount = 0;
        for (auto action : playlist->allmusic()) {
            if (searchStr.isEmpty()) {
                musicCount ++;
            } else {
                if (chineseFlag) {
                    if (action->title.contains(searchStr, Qt::CaseInsensitive)) {
                        musicCount++;
                    }
                } else {
                    if (searchStr.size() == 1) {
                        auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(action->title);
                        if (!curTextList.isEmpty() && curTextList.first().contains(searchStr, Qt::CaseInsensitive)) {
                            musicCount++;
                        }
                    } else {
                        auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(action->title);
                        if (!curTextList.isEmpty() && curTextList.join("").contains(searchStr, Qt::CaseInsensitive)) {
                            musicCount++;
                        }
                    }
                }
            }
        }
        if (musicCount == 0) {
            infoStr = MusicListDataWidget::tr("   No songs");
        } else if (musicCount == 1) {
            infoStr = MusicListDataWidget::tr("   1 song");
        } else {
            infoStr = MusicListDataWidget::tr("   %1 songs").arg(musicCount);
        }
        infoLabel->setText(infoStr);

        //update dropdown
        albumDropdown->hide();
        artistDropdown->hide();
        musicDropdown->show();

        albumListView->hide();
        artistListView->hide();
        musicListView->show();

        t_curDropdown = musicDropdown;

        if (musicListView->viewMode() != (playlist->viewMode())) {
            musicListView->setViewModeFlag((QListView::ViewMode)playlist->viewMode());
        }

        if (musicListView->viewMode() == QListView::IconMode) {
            btIconMode->setChecked(true);
            btlistMode->setChecked(false);
        } else {
            btIconMode->setChecked(false);
            btlistMode->setChecked(true);
        }
        musicListView->onMusiclistChanged(playlist);
    }

    if (playlist->sortType() == Playlist::SortByCustom) {
        q->setCustomSortType(playlist);
    } else {
        for (auto action : t_curDropdown->actions()) {
            if (action->data().toInt() == playlist->sortType()) {
                t_curDropdown->setCurrentAction(action);
            }
        }
    }
    showEmptyHits();
}

void MusicListDataWidgetPrivate::initConntion()
{
    Q_Q(MusicListDataWidget);

    q->connect(albumDropdown, &DDropdown::triggered,
    q, [ = ](QAction * action) {
        albumDropdown->setCurrentAction(action);
        int t_sortType = action->data().toInt() == 0 ? 1 : 0;
        albumListView->playlist()->changePlayMusicTypeOrderType();
        albumListView->playlist()->sortPlayMusicTypePtrListData(t_sortType);
        Q_EMIT q->resort(albumListView->playlist(), action->data().value<Playlist::SortType>());
    });
    q->connect(artistDropdown, &DDropdown::triggered,
    q, [ = ](QAction * action) {
        artistDropdown->setCurrentAction(action);
        int t_sortType = action->data().toInt() == 0 ? 1 : 0;
        artistListView->playlist()->changePlayMusicTypeOrderType();
        artistListView->playlist()->sortPlayMusicTypePtrListData(t_sortType);
        Q_EMIT q->resort(artistListView->playlist(), action->data().value<Playlist::SortType>());
    });
    q->connect(musicDropdown, &DDropdown::triggered,
    q, [ = ](QAction * action) {
        musicDropdown->setCurrentAction(action);
        Q_EMIT q->resort(musicListView->playlist(), action->data().value<Playlist::SortType>());
    });

    q->connect(btPlayAll, &DPushButton::clicked,
    q, [ = ](bool) {
        if (albumListView->isVisible()) {
            PlaylistPtr curPlayList = albumListView->playlist();
            if (curPlayList) {
                curPlayList->playMusicTypeToMeta();
                curPlayList->play(curPlayList->first());
                Q_EMIT q->playall(curPlayList);
                initData(curPlayList);
            }
        } else if (artistListView->isVisible()) {
            PlaylistPtr curPlayList = artistListView->playlist();
            if (curPlayList) {
                curPlayList->playMusicTypeToMeta();
                curPlayList->play(curPlayList->first());
                Q_EMIT q->playall(curPlayList);
                initData(curPlayList);
            }
        } else {
            if (musicListView->playlist()) {
                PlaylistPtr curPlayList = musicListView->playlist();
                curPlayList->play(curPlayList->first());
                Q_EMIT q->playall(curPlayList);
                initData(curPlayList);
            }
        }
    });

    //albumListView
    q->connect(albumListView, &MusicListDataView::requestCustomContextMenu,
    q, [ = ](const QPoint & pos) {
        Q_EMIT q->requestCustomContextMenu(pos, 2);
    });

    q->connect(albumListView, &MusicListDataView::playMedia,
    q, [ = ](const MetaPtr meta) {
        PlaylistPtr curPlayList = albumListView->playlist();
        curPlayList->play(meta);
        initData(curPlayList);
        Q_EMIT q->playMedia(albumListView->playlist(), meta);
    });
    q->connect(albumListView, &MusicListDataView::resume,
    q, [ = ](const MetaPtr meta) {
        PlaylistPtr curPlayList = albumListView->playlist();
        Q_EMIT q->resume(albumListView->playlist(), meta);
    });

    q->connect(albumListView, &MusicListDataView::pause,
    q, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        Q_EMIT q->pause(playlist, meta);
    });

    q->connect(albumListView, &MusicListDataView::addToPlaylist,
    q, [ = ](PlaylistPtr playlist, const MetaPtrList  & metalist) {
        Q_EMIT q->addToPlaylist(playlist, metalist);
    });

    q->connect(albumListView, &MusicListDataView::musiclistRemove,
    q, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT q->musiclistRemove(albumListView->playlist(), metalist);
    });

    q->connect(albumListView, &MusicListDataView::musiclistDelete,
    q, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT q->musiclistDelete(albumListView->playlist(), metalist);
    });

    q->connect(albumListView, &MusicListDataView::modeChanged,
    q, [ = ](int mode) {
        Q_EMIT q->modeChanged(mode);
    });

    //artistListView
    q->connect(artistListView, &MusicListDataView::requestCustomContextMenu,
    q, [ = ](const QPoint & pos) {
        Q_EMIT q->requestCustomContextMenu(pos, 3);
    });

    q->connect(artistListView, &MusicListDataView::playMedia,
    q, [ = ](const MetaPtr meta) {
        PlaylistPtr curPlayList = artistListView->playlist();
        curPlayList->play(meta);
        initData(curPlayList);
        Q_EMIT q->playMedia(artistListView->playlist(), meta);
    });

    q->connect(artistListView, &MusicListDataView::resume,
    q, [ = ](const MetaPtr meta) {
        PlaylistPtr curPlayList = artistListView->playlist();
        Q_EMIT q->resume(artistListView->playlist(), meta);
    });

    q->connect(artistListView, &MusicListDataView::pause,
    q, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        Q_EMIT q->pause(playlist, meta);
    });

    q->connect(musicListView, &PlayListView::pause,
    q, [ = ](const MetaPtr meta) {
        Q_EMIT q->pause(musicListView->playlist(), meta);
    });

    q->connect(artistListView, &MusicListDataView::addToPlaylist,
    q, [ = ](PlaylistPtr playlist, const MetaPtrList  & metalist) {
        Q_EMIT q->addToPlaylist(playlist, metalist);
    });

    q->connect(artistListView, &MusicListDataView::musiclistRemove,
    q, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT q->musiclistRemove(artistListView->playlist(), metalist);
    });

    q->connect(artistListView, &MusicListDataView::musiclistDelete,
    q, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT q->musiclistDelete(artistListView->playlist(), metalist);
    });

    q->connect(artistListView, &MusicListDataView::modeChanged,
    q, [ = ](int mode) {
        Q_EMIT q->modeChanged(mode);
    });

    //musicListView
    q->connect(musicListView, &PlayListView::playMedia,
    q, [ = ](const MetaPtr meta) {
        PlaylistPtr curPlayList = musicListView->playlist();
        curPlayList->play(meta);
        //initData(curPlayList);
        Q_EMIT q->playMedia(musicListView->playlist(), meta);
    });

    q->connect(musicListView, &PlayListView::resume,
    q, [ = ](const MetaPtr meta) {
        PlaylistPtr curPlayList = musicListView->playlist();
        Q_EMIT q->resume(musicListView->playlist(), meta);
    });

    q->connect(musicListView, &PlayListView::pause,
    q, [ = ](const MetaPtr meta) {
        Q_EMIT q->pause(musicListView->playlist(), meta);
    });

    q->connect(musicListView, &PlayListView::requestCustomContextMenu,
    q, [ = ](const QPoint & pos) {
        Q_EMIT q->requestCustomContextMenu(pos, 1);
    });
    q->connect(musicListView, &PlayListView::addToPlaylist,
    q, [ = ](PlaylistPtr playlist, const MetaPtrList  metalist) {
        Q_EMIT q->addToPlaylist(playlist, metalist);
    });
    q->connect(musicListView, &PlayListView::removeMusicList,
    q, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT q->musiclistRemove(musicListView->playlist(), metalist);
    });
    q->connect(musicListView, &PlayListView::deleteMusicList,
    q, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT q->musiclistDelete(musicListView->playlist(), metalist);
    });
    q->connect(musicListView, &PlayListView::showInfoDialog,
    q, [ = ](const MetaPtr meta) {
        Q_EMIT q->showInfoDialog(meta);
    });
    q->connect(musicListView, &PlayListView::updateMetaCodec,
    q, [ = ](const MetaPtr  meta) {
        Q_EMIT q->updateMetaCodec(meta);
    });

    q->connect(btIconMode, &DPushButton::clicked,
    q, [ = ](bool) {
        if (albumListView->isVisible()) {
            albumListView->setViewModeFlag(QListView::IconMode);
        } else if (artistListView->isVisible()) {
            artistListView->setViewModeFlag(QListView::IconMode);
        } else {
            musicListView->playlist()->setViewMode(1);
            musicListView->setViewModeFlag(QListView::IconMode);
        }
        btIconMode->setChecked(true);
        btlistMode->setChecked(false);
    });
    q->connect(btlistMode, &DPushButton::clicked,
    q, [ = ](bool) {
        if (albumListView->isVisible()) {
            albumListView->setViewModeFlag(QListView::ListMode);
        } else if (artistListView->isVisible()) {
            artistListView->setViewModeFlag(QListView::ListMode);
        } else {
            musicListView->playlist()->setViewMode(0);
            musicListView->setViewModeFlag(QListView::ListMode);
        }
        btIconMode->setChecked(false);
        btlistMode->setChecked(true);
    });
}

void MusicListDataWidgetPrivate::showEmptyHits()
{
    if (!curPlaylist.isNull() && curPlaylist->id() == AlbumMusicListID) {
        auto playlist = artistListView->playlist();
        if (playlist.isNull() || playlist->id() != SearchMusicListID) {
            emptyHits->setText(MusicListDataWidget::tr("No Music"));
        } else {
            emptyHits->setText(MusicListDataWidget::tr("No result found"));
        }
        bool empty = false;//playlist.isNull();
        actionBar->setVisible(!empty);
        albumListView->setVisible(!empty);
        artistListView->setVisible(false);
        musicListView->setVisible(false);
        emptyHits->setVisible(empty);
    } else if (!curPlaylist.isNull() && curPlaylist->id() == ArtistMusicListID) {
        auto playlist = artistListView->playlist();
        if (playlist.isNull() || playlist->id() != SearchMusicListID) {
            emptyHits->setText(MusicListDataWidget::tr("No Music"));
        } else {
            emptyHits->setText(MusicListDataWidget::tr("No result found"));
        }
        bool empty = false;//playlist.isNull();
        actionBar->setVisible(!empty);
        artistListView->setVisible(!empty);
        albumListView->setVisible(false);
        musicListView->setVisible(false);
        emptyHits->setVisible(empty);
    } else {
        auto playlist = musicListView->playlist();
        if (playlist.isNull() || playlist->id() != SearchMusicListID) {
            emptyHits->setText(MusicListDataWidget::tr("No Music"));
        } else {
            emptyHits->setText(MusicListDataWidget::tr("No result found"));
        }
        bool empty = playlist.isNull();
        actionBar->setVisible(!empty);
        artistListView->setVisible(false);
        musicListView->setVisible(!empty);
        emptyHits->setVisible(empty);
    }
}

MusicListDataWidget::MusicListDataWidget(QWidget *parent) :
    DWidget(parent), d_ptr(new MusicListDataWidgetPrivate(this))
{
    Q_D(MusicListDataWidget);

    setObjectName("MusicListDataWidget");
    setAcceptDrops(true);

    setAutoFillBackground(true);
    auto palette = this->palette();
    QColor background("#FFFFFF");
    background.setAlphaF(0.1);
    palette.setColor(DPalette::Background, background);
    setPalette(palette);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 8, 0);
    layout->setSpacing(0);

    d->actionBar = new DWidget;
    d->actionBar->setFixedHeight(80);
    d->actionBar->setObjectName("MusicListDataActionBar");

    auto actionBarLayout = new QVBoxLayout(d->actionBar);
    actionBarLayout->setContentsMargins(10, 0, 8, 0);
    actionBarLayout->setSpacing(0);

    auto actionTileBarLayout = new QHBoxLayout();
    actionTileBarLayout->setContentsMargins(10, 0, 8, 0);
    actionTileBarLayout->setSpacing(0);

    d->titleLabel = new DLabel;
    auto titleFont = d->titleLabel->font();
    titleFont.setFamily("SourceHanSansSC");
    titleFont.setWeight(QFont::Medium);
    titleFont.setPixelSize(24);
    d->titleLabel->setFont(titleFont);
    d->titleLabel->setFixedHeight(36);
    d->titleLabel->setFixedWidth(300);
    d->titleLabel->setObjectName("MusicListDataTitle");
    d->titleLabel->setText(tr("All Music"));

    d->albumDropdown = new DDropdown;
    d->albumDropdown->setFixedHeight(28);
    d->albumDropdown->setMinimumWidth(130);
    d->albumDropdown->setObjectName("MusicListAlbumDataSort");
    d->albumDropdown->addAction(tr("Time added"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByAddTime));
    d->albumDropdown->addAction(tr("Album name"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByTitle));
    d->albumDropdown->setCurrentAction();
    d->albumDropdown->hide();

    d->artistDropdown = new DDropdown;
    d->artistDropdown->setFixedHeight(28);
    d->artistDropdown->setMinimumWidth(130);
    d->artistDropdown->setObjectName("MusicListArtistDataSort");
    d->artistDropdown->addAction(tr("Time added"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByAddTime));
    d->artistDropdown->addAction(tr("Artist name"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByArtist));
    d->artistDropdown->setCurrentAction();
    d->artistDropdown->hide();

    d->musicDropdown = new DDropdown;
    d->musicDropdown->setFixedHeight(28);
    d->musicDropdown->setMinimumWidth(130);
    d->musicDropdown->setObjectName("MusicListMusicDataSort");
    d->musicDropdown->addAction(tr("Time added"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByAddTime));
    d->musicDropdown->addAction(tr("Title"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByTitle));
    d->musicDropdown->addAction(tr("Artist name"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByArtist));
    d->musicDropdown->addAction(tr("Album name"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByAblum));
    d->musicDropdown->setCurrentAction();

    actionTileBarLayout->addWidget(d->titleLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    actionTileBarLayout->addStretch();
    actionTileBarLayout->addWidget(d->albumDropdown, 0, Qt::AlignCenter);
    actionTileBarLayout->addWidget(d->artistDropdown, 0, Qt::AlignCenter);
    actionTileBarLayout->addWidget(d->musicDropdown, 0, Qt::AlignCenter);

    auto actionInfoBarLayout = new QHBoxLayout();
    actionInfoBarLayout->setContentsMargins(10, 0, 8, 0);
    actionInfoBarLayout->setSpacing(0);

    d->btPlayAll = new DPushButton;
    auto playAllPalette = d->btPlayAll->palette();
    playAllPalette.setColor(DPalette::ButtonText, Qt::white);
    playAllPalette.setColor(DPalette::Dark, QColor("#FD5E5E"));
    playAllPalette.setColor(DPalette::Light, QColor("#ED5656"));
    d->btPlayAll->setPalette(playAllPalette);
    d->btPlayAll->setIcon(QIcon(":/mpimage/light/normal/play_all_normal.svg"));
    d->btPlayAll->setObjectName("MusicListDataPlayAll");
    d->btPlayAll->setText(tr("Play All"));
    d->btPlayAll->setFixedHeight(30);
    d->btPlayAll->setFocusPolicy(Qt::NoFocus);
    auto btPlayAllFont = d->btPlayAll->font();
    btPlayAllFont.setFamily("SourceHanSansSC-Medium");
    btPlayAllFont.setPixelSize(14);
    d->btPlayAll->setFont(btPlayAllFont);

    d->infoLabel = new DLabel;
    d->infoLabel->setObjectName("MusicListDataTitle");
    d->infoLabel->setText(tr("All Music"));
    d->infoLabel->setFont(btPlayAllFont);
    auto infoLabelPalette = d->infoLabel->palette();
    infoLabelPalette.setColor(DPalette::ButtonText, ("#777777"));
    d->infoLabel->setPalette(infoLabelPalette);

    d->btIconMode = new MusicImageButton(":/mpimage/light/normal/picture_list_normal.svg",
                                         ":/mpimage/light/hover/picture_list_hover.svg",
                                         ":/mpimage/light/press/picture_list_press.svg",
                                         ":/mpimage/light/active/picture_list_active.svg");
    d->btIconMode->setFixedSize(36, 36);
    d->btIconMode->setObjectName("MusicListDataWidgetIconMode");
    d->btIconMode->setCheckable(true);
    d->btIconMode->setChecked(true);

    d->btlistMode = new MusicImageButton(":/mpimage/light/normal/text_list_normal.svg",
                                         ":/mpimage/light/hover/text_list_hover.svg",
                                         ":/mpimage/light/press/text_list_press.svg",
                                         ":/mpimage/light/active/text_list_active.svg");
    d->btlistMode->setFixedSize(36, 36);
    d->btlistMode->setObjectName("MusicListDataWidgetListMode");
    d->btlistMode->setCheckable(true);

    actionInfoBarLayout->addWidget(d->btPlayAll, 0, Qt::AlignVCenter);
    actionInfoBarLayout->addWidget(d->infoLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    actionInfoBarLayout->addStretch();
    actionInfoBarLayout->addWidget(d->btIconMode, 0, Qt::AlignCenter);
    actionInfoBarLayout->addWidget(d->btlistMode, 0, Qt::AlignCenter);

    actionBarLayout->addLayout(actionTileBarLayout);
    actionBarLayout->addLayout(actionInfoBarLayout);

    d->emptyHits = new DLabel();
    d->emptyHits->setObjectName("MusicListDataEmptyHits");
    d->emptyHits->hide();

    d->albumListView = new MusicListDataView;
    d->artistListView = new MusicListDataView;
    d->musicListView = new PlayListView;
    d->musicListView->hide();

    layout->addWidget(d->actionBar, 0, Qt::AlignTop);
    layout->addWidget(d->emptyHits, 100, Qt::AlignCenter);
    layout->addWidget(d->albumListView, 100);
    layout->addWidget(d->artistListView, 100);
    layout->addWidget(d->musicListView, 100);

    d->initConntion();
}

MusicListDataWidget::~MusicListDataWidget()
{
}

void MusicListDataWidget::setCustomSortType(PlaylistPtr playlist)
{
    Q_D(MusicListDataWidget);

    DDropdown *t_curDropdown = nullptr;
    if (playlist->id() == AlbumMusicListID) {
        t_curDropdown = d->albumDropdown;
    } else if (playlist->id() == ArtistMusicListID) {
        t_curDropdown = d->artistDropdown;
    } else {
        t_curDropdown = d->musicDropdown;
    }
    t_curDropdown->setCurrentAction(nullptr);
    t_curDropdown->setText(tr("Custom"));
}

void MusicListDataWidget::onMusiclistChanged(PlaylistPtr playlist)
{
    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }

    Q_D(MusicListDataWidget);

    playlist->setSearchStr("");
    d->initData(playlist);
}

void MusicListDataWidget::onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_D(MusicListDataWidget);
    d->initData(d->curPlaylist);
}

void MusicListDataWidget::onMusiclistUpdate()
{
    Q_D(MusicListDataWidget);
    d->initData(d->curPlaylist);

    int t_count = 0;
    if (d->curPlaylist->id() == AlbumMusicListID ) {
        t_count = d->albumListView->playMusicTypePtrList().size();
    } else if (d->curPlaylist->id() == ArtistMusicListID) {
        t_count = d->artistListView->playMusicTypePtrList().size();
    } else {
        t_count = d->musicListView->playMetaPtrList().size();
    }

    if (t_count == 0 && !d->curPlaylist->searchStr().isEmpty()) {
        d->emptyHits->setText(tr("No result found"));
        d->albumListView->setVisible(false);
        d->artistListView->setVisible(false);
        d->musicListView->setVisible(false);
        d->emptyHits->setVisible(true);
    } else {
        d->emptyHits->setText("");
        d->emptyHits->hide();
    }
}

void MusicListDataWidget::onMusicPlayed(PlaylistPtr playlist, const MetaPtr Meta)
{
    Q_D(MusicListDataWidget);
    d->albumListView->setPlaying(Meta);
    d->artistListView->setPlaying(Meta);
    d->musicListView->setPlaying(Meta);
    d->albumListView->update();
    d->artistListView->update();
    d->musicListView->update();
}

void MusicListDataWidget::slotTheme(int type)
{
    Q_D(MusicListDataWidget);
    QString rStr;
    if (type == 1) {
        rStr = "light";
    } else {
        rStr = "dark";
    }
    if (type == 1) {
        auto palette = this->palette();
        QColor background("#FFFFFF");
        background.setAlphaF(0.1);
        palette.setColor(DPalette::Background, background);
        setPalette(palette);

        auto titleLabelPalette = d->infoLabel->palette();
        titleLabelPalette.setColor(DPalette::ButtonText, ("#000000"));
        d->titleLabel->setPalette(titleLabelPalette);
        d->titleLabel->setForegroundRole(DPalette::ButtonText);

        auto playAllPalette = d->btPlayAll->palette();
        playAllPalette.setColor(DPalette::ButtonText, Qt::white);
        playAllPalette.setColor(DPalette::Dark, QColor("#FD5E5E"));
        playAllPalette.setColor(DPalette::Light, QColor("#ED5656"));
        d->btPlayAll->setPalette(playAllPalette);

        auto infoLabelPalette = d->infoLabel->palette();
        infoLabelPalette.setColor(DPalette::ButtonText, ("#777777"));
        d->infoLabel->setPalette(infoLabelPalette);
        d->infoLabel->setForegroundRole(DPalette::ButtonText);

//        auto playAllPalette = d->btPlayAll->palette();
//        playAllPalette.setColor(DPalette::Dark, QColor("#FD5E5E"));
//        playAllPalette.setColor(DPalette::Light, QColor("#ED5656"));
//        d->btPlayAll->setPalette(palette);
    } else {
        auto palette = this->palette();
        QColor background("#252525");
        palette.setColor(DPalette::Background, background);
        setPalette(palette);
        auto titleLabelPalette = d->infoLabel->palette();
        titleLabelPalette.setColor(DPalette::ButtonText, ("#FFFFFF"));
        d->titleLabel->setPalette(titleLabelPalette);
        d->titleLabel->setForegroundRole(DPalette::ButtonText);

        auto playAllPalette = d->btPlayAll->palette();
        playAllPalette.setColor(DPalette::ButtonText, "#FFFFFF");
        playAllPalette.setColor(DPalette::Dark, QColor("#A51B1B"));
        playAllPalette.setColor(DPalette::Light, QColor("#DA2D2D"));
        d->btPlayAll->setPalette(playAllPalette);

        auto infoLabelPalette = d->infoLabel->palette();
        infoLabelPalette.setColor(DPalette::ButtonText, ("#C0C6D4"));
        d->infoLabel->setPalette(infoLabelPalette);
        d->infoLabel->setForegroundRole(DPalette::ButtonText);

//        auto playAllPalette = d->btPlayAll->palette();
//        playAllPalette.setColor(DPalette::Dark, QColor("#DA2D2D"));
//        playAllPalette.setColor(DPalette::Light, QColor("#A51B1B"));
//        d->btPlayAll->setPalette(palette);
    }

    d->btIconMode->setPropertyPic(QString(":/mpimage/%1/normal/picture_list_normal.svg").arg(rStr),
                                  QString(":/mpimage/%1/hover/picture_list_hover.svg").arg(rStr),
                                  QString(":/mpimage/%1/press/picture_list_press.svg").arg(rStr),
                                  QString(":/mpimage/%1/active/picture_list_active.svg").arg(rStr));
    d->btlistMode->setPropertyPic(QString(":/mpimage/%1/normal/text_list_normal.svg").arg(rStr),
                                  QString(":/mpimage/%1/hover/text_list_hover.svg").arg(rStr),
                                  QString(":/mpimage/%1/press/text_list_press.svg").arg(rStr),
                                  QString(":/mpimage/%1/active/text_list_active.svg").arg(rStr));
    d->albumListView->setThemeType(type);
    d->artistListView->setThemeType(type);
    d->musicListView->setThemeType(type);
}

void MusicListDataWidget::changePicture(QPixmap pixmap)
{
    Q_D(MusicListDataWidget);
    if (d->albumListView->isVisible()) {
        d->albumListView->setPlayPixmap(pixmap);
    } else if (d->artistListView->isVisible()) {
        d->artistListView->setPlayPixmap(pixmap);
    } else {
        d->musicListView->setPlayPixmap(pixmap);
    }
}

void MusicListDataWidget::onCustomContextMenuRequest(const QPoint &pos, PlaylistPtr selectedlist, PlaylistPtr favlist, QList<PlaylistPtr> newlists, char type)
{
    Q_D(MusicListDataWidget);
    if (type == 2) {
        d->albumListView->showContextMenu(pos, d->musicListView->playlist(), favlist, newlists);
    } else if (type == 3) {
        d->artistListView->showContextMenu(pos, d->musicListView->playlist(), favlist, newlists);
    } else {
        d->musicListView->showContextMenu(pos, d->musicListView->playlist(), favlist, newlists);
    }
}

