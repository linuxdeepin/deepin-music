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

#include "musiclistinfoview.h"
#include "musicimagebutton.h"
#include "infodialog.h"

DWIDGET_USE_NAMESPACE

class MusicListDialogPrivate
{
public:
    MusicListDialogPrivate(MusicListDialog *parent) : q_ptr(parent) {}

    void initUI();
    void initConnection();

    DLabel              *titleFrame     = nullptr;
    DLabel              *titleLabel     = nullptr;
    DLabel              *infoLabel      = nullptr;
    DPushButton         *btPlayAll      = nullptr;
    DPushButton         *btRandomPlay   = nullptr;
    InfoDialog          *infoDialog     = nullptr;

    MusicListInfoView   *musicListInfoView   = nullptr;

    MusicListDialog *q_ptr;
    Q_DECLARE_PUBLIC(MusicListDialog)
};

void MusicListDialogPrivate::initUI()
{
    Q_Q(MusicListDialog);

    q->setAutoFillBackground(true);
    auto palette = q->palette();
    QColor BackgroundColor("#F8F8F8");
    palette.setColor(DPalette::Background, BackgroundColor);
    q->setPalette(palette);

    auto layout = new QVBoxLayout(q);
    layout->setSpacing(0);
    layout->setMargin(10);

    titleFrame = new DLabel;
    titleFrame->setFixedSize(480, 130);
    QPixmap pixmap = DHiDPIHelper::loadNxPixmap(":/common/image/cover_max.svg");
    pixmap = pixmap.scaled(480, 130, Qt::KeepAspectRatioByExpanding);
    titleFrame->setPixmap(pixmap);

    auto titleLayout = new QVBoxLayout(titleFrame);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(28, 0, 0, 18);

    auto closeBt = new DImageButton;
    closeBt->setPixmap(QPixmap(":/mpimage/light/normal/close_normal.svg"));
    closeBt->setObjectName("InfoClose");
    closeBt->setFixedSize(27, 23);

    titleLabel = new DLabel();
    auto titleFont = titleLabel->font();
    titleFont.setPixelSize(24);
    titleLabel->setFont(titleFont);
    infoLabel = new DLabel();
    auto infoFont = titleLabel->font();
    infoFont.setPixelSize(18);
    infoLabel->setFont(infoFont);

    auto btLayout = new QHBoxLayout(titleFrame);
    btLayout->setSpacing(0);
    btLayout->setMargin(0);

    btPlayAll = new DPushButton;
    auto btPlayAllFont = btPlayAll->font();
    btPlayAllFont.setPixelSize(14);
    btPlayAll->setFont(btPlayAllFont);
    auto playAllPalette = btPlayAll->palette();
    playAllPalette.setColor(DPalette::ButtonText, Qt::white);
    playAllPalette.setColor(DPalette::Dark, QColor(Qt::red));
    playAllPalette.setColor(DPalette::Light, QColor(Qt::red));
    btPlayAll->setPalette(playAllPalette);
    btPlayAll->setIcon(QIcon(":/mpimage/light/normal/play_all_normal.svg"));
    btPlayAll->setText(MusicListDialog::tr("Play All"));
    btPlayAll->setFixedHeight(30);
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
    btRandomPlay->setFixedHeight(30);
    btRandomPlay->setFocusPolicy(Qt::NoFocus);

    btLayout->addWidget(btPlayAll);
    btLayout->addWidget(btRandomPlay);
    btLayout->addStretch(100);

    titleLayout->addWidget(closeBt, 0, Qt::AlignTop | Qt::AlignRight);
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(infoLabel);
    titleLayout->addLayout(btLayout);

    musicListInfoView = new MusicListInfoView;
    musicListInfoView->setFixedWidth(480);

    layout->addWidget(titleFrame, 0, Qt::AlignTop | Qt::AlignRight);
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
        if (musicListInfoView->playlist()->first() != nullptr) {
            Q_EMIT q->playMedia(musicListInfoView->playlist()->first());
            Q_EMIT q->modeChanged(0);
        }
    });

    q->connect(btRandomPlay, &DPushButton::clicked,
    q, [ = ]() {
        musicListInfoView->playlist()->playMusicTypeToMeta(musicListInfoView->curName());
        if (musicListInfoView->playlist()->first() != nullptr) {
            Q_EMIT q->playMedia(musicListInfoView->playlist()->first());
            Q_EMIT q->modeChanged(2);
        }
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
}

MusicListDialog::MusicListDialog(QWidget *parent)
    : DAbstractDialog(parent), d_ptr(new MusicListDialogPrivate(this))
{
    Q_D(MusicListDialog);
    setFixedSize(500, 500);

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
    if (!playMusicType->extraName.isEmpty()) {
        titleFont.setPixelSize(24);
        d->infoLabel->setText(playMusicType->extraName);
    } else {
        titleFont.setPixelSize(36);
        d->infoLabel->hide();
    }
    d->titleLabel->setFont(titleFont);

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
    } else {
        auto palette = this->palette();
        QColor BackgroundColor("#252525");
        palette.setColor(DPalette::Background, BackgroundColor);
        setPalette(palette);
    }
    d->musicListInfoView->setThemeType(type);
    d->infoDialog->setThemeType(type);
}
