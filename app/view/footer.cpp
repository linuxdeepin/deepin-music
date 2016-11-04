/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "footer.h"

#include <QTimer>
#include <QStyle>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QProgressBar>

#include <dthememanager.h>
DWIDGET_USE_NAMESPACE

#include "../musicapp.h"
#include "../core/playlistmanager.h"

#include "widget/slider.h"
#include "widget/modebuttom.h"

static const char *sPropertyFavourite         = "fav";
static const char *sPropertyPlayStatus        = "playstatus";

static const QString sPlayStatusValuePlaying    = "playing";
static const QString sPlayStatusValuePause      = "pause";
static const QString sPlayStatusValueStop       = "stop";

ClickableLabel::ClickableLabel(const QString &text, QWidget *parent)
    : QLabel(parent)
{
    setText(text);
}

ClickableLabel::~ClickableLabel()
{
}

void ClickableLabel::mousePressEvent(QMouseEvent * /*event*/)
{
    emit clicked(false);
}

class FooterPrivate
{
public:
    ClickableLabel      *cover   = nullptr;
    ClickableLabel      *title   = nullptr;
    ClickableLabel      *artlist = nullptr;
    QPushButton *btPlay  = nullptr;
    QPushButton *btPrev  = nullptr;
    QPushButton *btNext  = nullptr;
    QPushButton *btFavorite = nullptr;
    QPushButton *btLyric    = nullptr;
    ModeButton *btPlayMode = nullptr;
    QPushButton *btSound    = nullptr;
    Slider     *progress  = nullptr;
    Slider      *hideProgress = nullptr;

    QSharedPointer<Playlist>    m_playinglist;
    MusicMeta                   m_info;
    int                         m_mode;
};

Footer::Footer(QWidget *parent) : QFrame(parent)
{
    setFocusPolicy(Qt::ClickFocus);
    d = QSharedPointer<FooterPrivate>(new FooterPrivate);
    setObjectName("Footer");

    auto vboxlayout = new QVBoxLayout(this);
    vboxlayout->setSpacing(0);
    vboxlayout->setMargin(0);

    d->progress = new Slider(Qt::Horizontal);
    d->progress->setObjectName("FooterProgress");
    d->progress->setFixedHeight(2);
    d->progress->setMinimum(0);
    d->progress->setMaximum(1000);
    d->progress->setValue(0);

    d->hideProgress = new Slider(Qt::Horizontal);
    d->hideProgress->setObjectName("HideFooterProgress");
    d->hideProgress->setFixedHeight(10);
    d->hideProgress->setMinimum(0);
    d->hideProgress->setMaximum(1000);
    d->hideProgress->setValue(0);

    auto layout = new QHBoxLayout();
    layout->setContentsMargins(20, 0, 20, 10);
    layout->setSpacing(20);

    d->cover = new ClickableLabel;
    d->cover->setObjectName("FooterCover");
    d->cover->setFixedSize(40, 40);

    d->title = new ClickableLabel;
    d->title->setObjectName("FooterTitle");
    d->title->setMaximumWidth(240);
    d->title->setText(tr("Unknow Title"));

    d->artlist = new ClickableLabel;
    d->artlist->setObjectName("FooterArtlist");
    d->artlist->setMaximumWidth(240);
    d->artlist->setText(tr("Unknow Artlist"));

    d->btPlay = new QPushButton;
    d->btPlay->setObjectName("FooterActionPlay");
    d->btPlay->setFixedSize(30, 30);

    d->btPrev = new QPushButton;
    d->btPrev->setObjectName("FooterActionPrev");
    d->btPrev->setFixedSize(30, 30);

    d->btNext = new QPushButton;
    d->btNext->setObjectName("FooterActionNext");
    d->btNext->setFixedSize(30, 30);

    d->btFavorite = new QPushButton;
    d->btFavorite->setObjectName("FooterActionFavorite");
    d->btFavorite->setFixedSize(24, 24);

    d->btLyric = new QPushButton;
    d->btLyric->setObjectName("FooterActionLyric");
    d->btLyric->setFixedSize(24, 24);

    QStringList modes;
    modes /*<< ":/image/sequence"*/
            << ":/image/repeat_all"
            << ":/image/repeat_single"
            << ":/image/shuffle";
    d->btPlayMode = new ModeButton;
    d->btPlayMode->setObjectName("FooterActionPlayMode");
    d->btPlayMode->setFixedSize(24, 24);
    d->btPlayMode->setModeIcons(modes);

    d->btSound = new QPushButton;
    d->btSound->setObjectName("FooterActionSound");
    d->btSound->setFixedSize(24, 24);

    auto btPlayList = new QPushButton;
    btPlayList->setObjectName("FooterActionPlayList");
    btPlayList->setFixedSize(24, 24);

    auto infoWidget = new QWidget;
    auto infoLayout = new QHBoxLayout(infoWidget);
    auto musicMetaLayout = new QVBoxLayout;
    musicMetaLayout->addWidget(d->title);
    musicMetaLayout->addWidget(d->artlist);
    musicMetaLayout->setSpacing(0);
    infoLayout->setMargin(0);
    infoLayout->addWidget(d->cover, 0, Qt::AlignLeft | Qt::AlignVCenter);
    infoLayout->addLayout(musicMetaLayout, 0);

    auto ctlWidget = new QWidget;
    auto ctlLayout = new QHBoxLayout(ctlWidget);
    ctlLayout->setMargin(0);
    ctlLayout->setSpacing(30);
    ctlLayout->addWidget(d->btPrev, 0, Qt::AlignCenter);
    ctlLayout->addWidget(d->btPlay, 0, Qt::AlignCenter);
    ctlLayout->addWidget(d->btNext, 0, Qt::AlignCenter);

    auto actWidget = new QWidget;
    auto actLayout = new QHBoxLayout(actWidget);
    actLayout->setMargin(0);
    actLayout->setSpacing(20);
    actLayout->addWidget(d->btFavorite, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btLyric, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btPlayMode, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btSound, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(btPlayList, 0, Qt::AlignRight | Qt::AlignVCenter);

    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp.setHorizontalStretch(33);
    infoWidget->setSizePolicy(sp);
    ctlWidget->setSizePolicy(sp);
    actWidget->setSizePolicy(sp);

    layout->addWidget(infoWidget, 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addStretch();
    layout->addWidget(ctlWidget, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(actWidget, 0, Qt::AlignRight | Qt::AlignVCenter);

    vboxlayout->addWidget(d->progress);
    vboxlayout->addWidget(d->hideProgress);
    vboxlayout->addLayout(layout);

    d->title->hide();
    d->artlist->hide();
    d->btPrev->hide();
    d->btNext->hide();
    d->btFavorite->hide();
    d->btLyric->hide();

    D_THEME_INIT_WIDGET(Footer);

    connect(d->btPlayMode, &ModeButton::modeChanged, this, &Footer::modeChanged);
    connect(d->progress, &Slider::valueChanged, this, [ = ](int value) {
        auto range = d->progress->maximum() - d->progress->minimum();
        Q_ASSERT(range != 0);
        emit this->changeProgress(value, range);
    });
    connect(d->hideProgress, &Slider::valueChanged, this, [ = ](int value) {
        auto range = d->progress->maximum() - d->progress->minimum();
        Q_ASSERT(range != 0);
        emit this->changeProgress(value, range);
    });

    connect(d->btPlay, &QPushButton::clicked, this, [ = ](bool) {
        if (!d->m_playinglist) {
            emit play(d->m_playinglist, d->m_info);
            return;
        }

        auto status = d->btPlay->property(sPropertyPlayStatus).toString();
        if (status == sPlayStatusValuePlaying) {
            emit pause(d->m_playinglist, d->m_info);
            auto status = sPlayStatusValuePause;
            updateQssProperty(d->btPlay, sPropertyPlayStatus, status);
        } else {
            emit play(d->m_playinglist, d->m_info);
            auto status = sPlayStatusValuePlaying;
            updateQssProperty(d->btPlay, sPropertyPlayStatus, status);
        }
    });

    connect(d->btPrev, &QPushButton::clicked, this, [ = ](bool) {
        emit prev(d->m_playinglist, d->m_info);
    });
    connect(d->btNext, &QPushButton::clicked, this, [ = ](bool) {
        emit next(d->m_playinglist, d->m_info);
    });

    connect(d->btFavorite, &QPushButton::clicked, this, [ = ](bool) {
        emit toggleFavourite(d->m_info);
    });

    connect(d->cover, &ClickableLabel::clicked, d->btLyric, &QPushButton::clicked);
    connect(d->btLyric, &QPushButton::clicked, this, [ = ](bool) {
        emit  this->toggleLyric();
    });
    connect(btPlayList, &QPushButton::clicked, this, [ = ](bool) {
        emit  this->togglePlaylist();
    });

    // TODO: remove fav?
    connect(this, &Footer::initFooter,
    this, [ = ](QSharedPointer<Playlist> current, int mode) {
        d->m_mode = mode;
        d->m_playinglist = current;
        d->btPlayMode->setMode(mode);
        this->style()->unpolish(d->btPlayMode);
        this->style()->polish(d->btPlayMode);
        this->repaint();
    });

    QTimer::singleShot(200, this, [ = ] {
//        d->btPlay->setFocus();
    });
}

void Footer::enableControl(bool enable)
{
    d->btPrev->setEnabled(enable);
    d->btNext->setEnabled(enable);
    d->btFavorite->setEnabled(enable);
    d->btLyric->setEnabled(enable);
    d->btPlay->setEnabled(enable);
    d->btPlayMode->setEnabled(enable);
    d->cover->blockSignals(!enable);
}

void Footer::onMusicAdded(QSharedPointer<Playlist> palylist, const MusicMeta &info)
{
    if (palylist->id() == FavMusicListID)
        if (info.hash == d->m_info.hash) {
            updateQssProperty(d->btFavorite, sPropertyFavourite, true);
        }
}

void Footer::onMusicListAdded(QSharedPointer<Playlist> palylist, const MusicMetaList &infolist)
{
    if (palylist->id() == FavMusicListID)
        for (auto &meta : infolist) {
            if (meta.hash == d->m_info.hash) {
                updateQssProperty(d->btFavorite, sPropertyFavourite, true);
            }
        }
}

void Footer::onMusicRemoved(QSharedPointer<Playlist> palylist, const MusicMeta &info)
{
    if (palylist->id() == FavMusicListID)
        if (info.hash == d->m_info.hash) {
            updateQssProperty(d->btFavorite, sPropertyFavourite, false);
        }
}

void Footer::onMusicPlayed(QSharedPointer<Playlist> palylist, const MusicMeta &info)
{
    d->title->setText(info.title);
    d->artlist->setText(info.artist);

    d->title->show();
    d->artlist->show();
    d->btPrev->show();
    d->btNext->show();
    d->btFavorite->show();
    d->btLyric->show();

    d->m_playinglist = palylist;
    d->m_info = info;
    updateQssProperty(d->btFavorite, sPropertyFavourite, info.favourite);

    updateQssProperty(this, sPropertyPlayStatus, sPlayStatusValuePlaying);
    updateQssProperty(d->btPlay, sPropertyPlayStatus, sPlayStatusValuePlaying);
}

void Footer::onMusicPause(QSharedPointer<Playlist> palylist, const MusicMeta &info)
{
    if (info.hash != d->m_info.hash || palylist != d->m_playinglist) {
        qWarning() << "can not pasue" << d->m_playinglist << palylist
                   << d->m_info.hash << info.hash;
        return;
    }
    auto status = sPlayStatusValuePause;
    updateQssProperty(d->btPlay, sPropertyPlayStatus, status);
}

//void Footer::onMusicStop(QSharedPointer<Playlist> palylist, const MusicMeta &info)
//{

//}

void Footer::onProgressChanged(qint64 value, qint64 duration)
{
    auto length = d->progress->maximum() - d->progress->minimum();
    Q_ASSERT(length != 0);
    Q_ASSERT(duration != 0);
    d->progress->blockSignals(true);
    d->progress->setValue(value * length / duration);
    d->progress->blockSignals(false);

    d->hideProgress->blockSignals(true);
    d->hideProgress->setValue(value * length / duration);
    d->hideProgress->blockSignals(false);
}

void Footer::onCoverChanged(const MusicMeta &info, const QString &coverPath)
{
    if (info.hash != d->m_info.hash) {
        return;
    }

    d->cover->setStyleSheet(
        QString("#FooterCover {image: url(%1) no-repeat center center fixed;}").arg(coverPath));
    this->style()->unpolish(d->cover);
    this->style()->polish(d->cover);
    d->cover->repaint();
}

void Footer::updateQssProperty(QWidget *w, const char *name, const QVariant &value)
{
    w->setProperty(name, value);
    this->style()->unpolish(w);
    this->style()->polish(w);
    w->repaint();
}
