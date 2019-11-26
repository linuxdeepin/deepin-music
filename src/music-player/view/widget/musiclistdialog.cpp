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

#include "musiclistdialog.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileInfo>
#include <DPalette>

#include <DFrame>
#include <DLabel>
#include <DImageButton>
#include <DPushButton>
#include <DHiDPIHelper>
#include <DApplicationHelper>

#include "musiclistinfoview.h"
#include "musicimagebutton.h"
#include "infodialog.h"
#include "musictitleimage.h"

DWIDGET_USE_NAMESPACE

class MusicListDialogPrivate
{
public:
    MusicListDialogPrivate(MusicListDialog *parent) : q_ptr(parent) {}

    void initUI();
    void initConnection();

    MusicTitleImageWidget              *titleFrame          = nullptr;
    DLabel                             *titleLabel          = nullptr;
    DLabel                             *infoLabel           = nullptr;
    DPushButton                        *btPlayAll           = nullptr;
    DPushButton                        *btRandomPlay        = nullptr;
    InfoDialog                         *infoDialog          = nullptr;
    DImageButton                       *closeBt             = nullptr;
    MusicListInfoView                  *musicListInfoView   = nullptr;

    MusicListDialog *q_ptr;
    Q_DECLARE_PUBLIC(MusicListDialog)
};

void MusicListDialogPrivate::initUI()
{
    Q_Q(MusicListDialog);

    q->setAutoFillBackground(true);
    auto palette = q->palette();
    QColor BackgroundColor("#F8F8F8");
    palette.setColor(DPalette::Window, BackgroundColor);
    q->setPalette(palette);

    auto layout = new QVBoxLayout(q);
    layout->setSpacing(0);
    layout->setMargin(10);

    titleFrame = new MusicTitleImageWidget;
    titleFrame->setFixedSize(480, 130);
    QPixmap pixmap = DHiDPIHelper::loadNxPixmap(":/common/image/cover_max.svg");
    pixmap = pixmap.scaled(480, 130, Qt::KeepAspectRatioByExpanding);
    titleFrame->setPixmap(pixmap);

    auto closeLayout = new QHBoxLayout(titleFrame);
    closeLayout->setSpacing(0);
    closeLayout->setContentsMargins(0, 5, 5, 0);

    auto titleLayout = new QVBoxLayout(titleFrame);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(28, 10, 0, 18);

    closeBt = new DImageButton;
    closeBt->setNormalPic(":/mpimage/light/normal/close_round normal.svg");
    closeBt->setHoverPic(":/mpimage/light/hover/close_round hover.svg");
    closeBt->setPressPic(":/mpimage/light/press/close_round press.svg");
    closeBt->setObjectName("InfoClose");
    closeBt->setBaseSize(24, 24);

    titleLabel = new DLabel();
    titleLabel->setForegroundRole(DPalette::TextTitle);

    infoLabel = new DLabel();
    infoLabel->setForegroundRole(DPalette::TextTitle);


    auto btLayout = new QHBoxLayout(titleFrame);
    btLayout->setSpacing(0);
    btLayout->setMargin(0);

    btPlayAll = new DPushButton;
    auto btPlayAllFont = btPlayAll->font();
    btPlayAllFont.setPixelSize(13);
    btPlayAll->setFont(btPlayAllFont);
    auto playAllPalette = btPlayAll->palette();
    playAllPalette.setColor(DPalette::ButtonText, Qt::white);
    playAllPalette.setColor(DPalette::Dark, QColor(Qt::red));
    playAllPalette.setColor(DPalette::Light, QColor(Qt::red));
    btPlayAll->setPalette(playAllPalette);
    btPlayAll->setIcon(QIcon(":/mpimage/light/normal/play_all_normal.svg"));
    btPlayAll->setText(MusicListDialog::tr("Play All"));
    btPlayAll->setFixedHeight(31);
    btPlayAll->setFocusPolicy(Qt::NoFocus);

    btRandomPlay = new DPushButton;
    btRandomPlay->setFont(btPlayAllFont);
    auto randomPlayPalette = btRandomPlay->palette();
    randomPlayPalette.setColor(DPalette::ButtonText, Qt::white);
    randomPlayPalette.setColor(DPalette::Dark, QColor(Qt::darkGray));
    randomPlayPalette.setColor(DPalette::Light, QColor(Qt::darkGray));
    btRandomPlay->setPalette(randomPlayPalette);
    btRandomPlay->setIcon(QIcon(":/mpimage/light/normal/random_play_normal.svg"));
    btRandomPlay->setText(MusicListDialog::tr("Shuffle Play"));
    btRandomPlay->setFixedHeight(31);
    btRandomPlay->setFocusPolicy(Qt::NoFocus);

    btLayout->addWidget(btPlayAll);
    btLayout->addSpacing(10);
    btLayout->addWidget(btRandomPlay);
    btLayout->addStretch(100);

//    titleLayout->addWidget(closeBt, 0, Qt::AlignTop | Qt::AlignRight);
    titleLayout->addWidget(titleLabel, 0, Qt::AlignTop);
    titleLayout->addWidget(infoLabel, 0, Qt::AlignTop);
    titleLayout->addLayout(btLayout, Qt::AlignTop);

    closeLayout->addLayout(titleLayout);
    closeLayout->addWidget(closeBt, 0, Qt::AlignTop | Qt::AlignRight);

    musicListInfoView = new MusicListInfoView;
//    DPalette pa = musicListInfoView->palette();
//    pa.setColor(DPalette::Base, pa.window().color());
//    musicListInfoView->setFrameShape(QFrame::NoFrame);
//    musicListInfoView->setPalette(pa);
//    musicListInfoView->setAutoFillBackground(true);
    musicListInfoView->setFixedWidth(479);

    layout->addWidget(titleFrame, 0, Qt::AlignTop | Qt::AlignCenter);
    layout->addWidget(musicListInfoView);

    infoDialog = new InfoDialog(q);
    infoDialog->hide();

    q->connect(closeBt, &DImageButton::clicked, q, &DAbstractDialog::close);
}

void MusicListDialogPrivate::initConnection()
{
    Q_Q(MusicListDialog);

    auto layout = new QVBoxLayout(q);
    layout->setSpacing(0);
    layout->setMargin(5);

    q->connect(musicListInfoView, &MusicListInfoView::requestCustomContextMenu,
    q, [ = ](const QPoint & pos) {
        Q_EMIT q->requestCustomContextMenu(pos);
    });

    q->connect(btPlayAll, &DPushButton::clicked,
    q, [ = ]() {
        musicListInfoView->playlist()->playMusicTypeToMeta(musicListInfoView->curName());
        auto curtMeta = musicListInfoView->firstMeta();
        if (curtMeta != nullptr) {
            Q_EMIT q->playMedia(curtMeta);
            Q_EMIT q->modeChanged(0);
        }
    });

    q->connect(btRandomPlay, &DPushButton::clicked,
    q, [ = ]() {
        musicListInfoView->playlist()->playMusicTypeToMeta(musicListInfoView->curName());
        if (musicListInfoView->playlist()->first() != nullptr) {

            auto curPlayList = musicListInfoView->playlist();
            bool invalidFlag = true;
            for (auto curMata : curPlayList->allmusic()) {
                if (!curMata->invalid) {
                    invalidFlag = false;
                    break;
                }
            }
            if (!invalidFlag) {
                auto cutMeta = musicListInfoView->playlist()->playing();
                while (true) {
                    cutMeta = musicListInfoView->playlist()->shuffleNext(cutMeta);
                    if (!cutMeta->invalid)
                        break;
                }

                Q_EMIT q->playMedia(cutMeta);
                Q_EMIT q->modeChanged(2);
            }
        }
    });

    q->connect(musicListInfoView, &MusicListInfoView::resume,
    q, [ = ](const MetaPtr meta) {
        Q_EMIT q->resume(meta);
    });

    q->connect(musicListInfoView, &MusicListInfoView::playMedia,
    q, [ = ](const MetaPtr meta) {
        musicListInfoView->playlist()->playMusicTypeToMeta(musicListInfoView->curName());
        Q_EMIT q->playMedia(meta);
        musicListInfoView->update();
    });

    q->connect(musicListInfoView, &MusicListInfoView::removeMusicList,
    q, [ = ](const MetaPtrList  & metalist) {
        QString name = musicListInfoView->curName();
        auto playlist = musicListInfoView->playlist();
        for (auto meta : metalist) {
            for (auto TypePtr : playlist->playMusicTypePtrList()) {
                if (TypePtr->name == name) {
                    if (TypePtr->playlistMeta.metas.contains(meta->hash)) {
                        TypePtr->playlistMeta.metas.remove(meta->hash);
                    }
                }
            }
        }
        musicListInfoView->onMusiclistChanged(playlist, name);

        Q_EMIT q->musiclistRemove(metalist);
    });
    q->connect(musicListInfoView, &MusicListInfoView::deleteMusicList,
    q, [ = ](const MetaPtrList & metalist) {
        QString name = musicListInfoView->curName();
        auto playlist = musicListInfoView->playlist();
        for (auto meta : metalist) {
            for (auto TypePtr : playlist->playMusicTypePtrList()) {
                if (TypePtr->name == name) {
                    if (TypePtr->playlistMeta.metas.contains(meta->hash)) {
                        TypePtr->playlistMeta.metas.remove(meta->hash);
                    }
                }
            }
        }
        musicListInfoView->onMusiclistChanged(playlist, name);

        Q_EMIT q->musiclistDelete(metalist);
    });
    q->connect(musicListInfoView, &MusicListInfoView::addToPlaylist,
    q, [ = ](PlaylistPtr playlist, const MetaPtrList  metalist) {
        Q_EMIT q->addToPlaylist(playlist, metalist);
    });

    q->connect(musicListInfoView, &MusicListInfoView::showInfoDialog,
    q, [ = ](const MetaPtr meta) {
        infoDialog->show();
        infoDialog->updateInfo(meta);
    });

    q->connect(musicListInfoView, &MusicListInfoView::pause,
    q, [ = ](const MetaPtr meta) {
        Q_EMIT q->pause(meta);
    });
    q->connect(musicListInfoView, &MusicListInfoView::addMetasFavourite,
    q, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT q->addMetasFavourite(metalist);
    });
    q->connect(musicListInfoView, &MusicListInfoView::removeMetasFavourite,
    q, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT q->removeMetasFavourite(metalist);
    });
}

MusicListDialog::MusicListDialog(QWidget *parent)
    : DAbstractDialog(parent), d_ptr(new MusicListDialogPrivate(this))
{
    Q_D(MusicListDialog);
    resize(500, 500);

    d->initUI();
    d->initConnection();
}

MusicListDialog::~MusicListDialog()
{

}

void MusicListDialog::setPlayMusicData(PlaylistPtr playlist, PlayMusicTypePtr playMusicType)
{
    Q_D(MusicListDialog);

    d->titleLabel->setText(playMusicType->name);
    auto titleFont = d->titleLabel->font();
    auto infoFont = d->infoLabel->font();
    if (!playMusicType->extraName.isEmpty()) {
        titleFont.setPixelSize(24);
        infoFont.setPixelSize(18);
        d->infoLabel->setText(playMusicType->extraName);
    } else {
        titleFont.setPixelSize(36);
        d->infoLabel->hide();
    }
    d->titleLabel->setFont(titleFont);
    d->infoLabel->setFont(infoFont);

    QString name = playMusicType->name;
    for (auto meta : playlist->playMusicTypePtrList()) {
        if (meta->name == name) {
            if (!meta->icon.isEmpty()) {
                QPixmap pixmap = QPixmap::fromImage(QImage::fromData(meta->icon));
                pixmap = pixmap.scaled(480, 130, Qt::KeepAspectRatioByExpanding);
                d->titleFrame->setPixmap(pixmap);
            } else {
                QPixmap pixmap = DHiDPIHelper::loadNxPixmap(":/common/image/cover_max.svg");
                pixmap = pixmap.scaled(480, 130, Qt::KeepAspectRatioByExpanding);
                d->titleFrame->setPixmap(pixmap);
            }
            break;
        }
    }

    d->musicListInfoView->onMusiclistChanged(playlist, name);
}

void MusicListDialog::setPlaying(const MetaPtr Meta)
{
    Q_D(MusicListDialog);
    d->musicListInfoView->setPlaying(Meta);
}

void MusicListDialog::showContextMenu(const QPoint &pos, PlaylistPtr selectedPlaylist, PlaylistPtr favPlaylist, QList<PlaylistPtr> newPlaylists)
{
    Q_D(MusicListDialog);
    d->musicListInfoView->showContextMenu(pos, selectedPlaylist, favPlaylist, newPlaylists);
}

void MusicListDialog::setThemeType(int type)
{
    Q_D(MusicListDialog);
    if (type == 1) {
        auto palette = this->palette();
        QColor BackgroundColor("#F8F8F8");
        palette.setColor(DPalette::Background, BackgroundColor);
        setPalette(palette);

//        d->closeBt->setNormalPic(":/mpimage/light/normal/close_round normal.svg");
//        d->closeBt->setHoverPic(":/mpimage/light/hover/close_round hover.svg");
//        d->closeBt->setPressPic(":/mpimage/light/press/close_round press.svg");


//        auto titleLabelPl = d->titleLabel->palette();
//        titleLabelPl.setColor(DPalette::WindowText, Qt::black);
//        d->titleLabel->setPalette(titleLabelPl);

//        auto infoLabelPl = d->infoLabel->palette();
//        infoLabelPl.setColor(DPalette::WindowText, Qt::black);
//        d->infoLabel->setPalette(infoLabelPl);

        auto playAllPalette = d->btPlayAll->palette();
        playAllPalette.setColor(DPalette::ButtonText, Qt::white);
        playAllPalette.setColor(DPalette::Dark, QColor("#FD5E5E"));
        playAllPalette.setColor(DPalette::Light, QColor("#FD5E5E"));
        QColor sbcolor("#F82C47");
        sbcolor.setAlphaF(0.4);
        playAllPalette.setColor(DPalette::Shadow, sbcolor);
        d->btPlayAll->setPalette(playAllPalette);

        auto randomPlayPalette = d->btRandomPlay->palette();
        randomPlayPalette.setColor(DPalette::ButtonText, Qt::white);
        randomPlayPalette.setColor(DPalette::Dark, QColor("#646464"));
        randomPlayPalette.setColor(DPalette::Light, QColor("#5C5C5C"));
        QColor randombcolor("#000000");
        randombcolor.setAlphaF(0.2);
        randomPlayPalette.setColor(DPalette::Shadow, randombcolor);
        d->btRandomPlay->setPalette(randomPlayPalette);
    } else {
        auto palette = this->palette();
        QColor BackgroundColor("#252525");
        palette.setColor(DPalette::Background, BackgroundColor);
        setPalette(palette);

//        d->closeBt->setNormalPic(":/mpimage/dark/close_round/close_normal_dark.svg");
//        d->closeBt->setHoverPic(":/mpimage/dark/close_round/close_hover_dark.svg");
//        d->closeBt->setPressPic(":/mpimage/dark/close_round/close_press.svg");

//        auto titleLabelPl = d->titleLabel->palette();
//        titleLabelPl.setColor(DPalette::WindowText, Qt::white);
//        d->titleLabel->setPalette(titleLabelPl);

//        auto infoLabelPl = d->infoLabel->palette();
//        infoLabelPl.setColor(DPalette::WindowText, Qt::white);
//        d->infoLabel->setPalette(infoLabelPl);

        auto playAllPalette = d->btPlayAll->palette();
        playAllPalette.setColor(DPalette::ButtonText, "#FFFFFF");
        playAllPalette.setColor(DPalette::Dark, QColor("#DA2D2D"));
        playAllPalette.setColor(DPalette::Light, QColor("#A51B1B"));
        QColor sbcolor("#C10A0A");
        sbcolor.setAlphaF(0.5);
        playAllPalette.setColor(DPalette::Shadow, sbcolor);
        d->btPlayAll->setPalette(playAllPalette);

        auto randomPlayPalette = d->btRandomPlay->palette();
        randomPlayPalette.setColor(DPalette::ButtonText, "#FFFFFF");
        randomPlayPalette.setColor(DPalette::Dark, QColor("#555454"));
        randomPlayPalette.setColor(DPalette::Light, QColor("#414141"));
//        QColor randombcolor("#FFFFFF");
//        randombcolor.setAlphaF(0.08);
//        randomPlayPalette.setColor(DPalette::Shadow, randombcolor);
        d->btRandomPlay->setPalette(randomPlayPalette);
    }
    QString rStr;
    if (type == 1) {
        rStr = "light";
    } else {
        rStr = "dark";
    }
//    d->closeBt->setPixmap(QPixmap(QString(":/mpimage/light/normal/close_round normal.svg").arg(rStr)));
    d->closeBt->setNormalPic(":/mpimage/light/normal/close_round normal.svg");
    d->closeBt->setHoverPic(":/mpimage/light/hover/close_round hover.svg");
    d->closeBt->setPressPic(":/mpimage/light/press/close_round press.svg");
    d->btPlayAll->setIcon(QIcon(QString(":/mpimage/light/normal/play_all_normal.svg").arg(rStr)));
    d->btRandomPlay->setIcon(QIcon(QString(":/mpimage/light/normal/random_play_normal.svg").arg(rStr)));
    d->titleFrame->setThemeType(type);
    d->musicListInfoView->setThemeType(type);
    d->infoDialog->setThemeType(type);
}

void MusicListDialog::setPlayPixmap(QPixmap pixmap)
{
    Q_D(MusicListDialog);
    d->musicListInfoView->setPlayPixmap(pixmap);
    update();
}
