/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "footer.h"

#include <QDebug>
#include <QTimer>
#include <QStyle>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QProgressBar>

#include <thememanager.h>

#include "../musicapp.h"
#include "../core/playlistmanager.h"

#include "widget/filter.h"
#include "widget/slider.h"
#include "widget/modebuttom.h"
#include "widget/label.h"
#include "widget/cover.h"
#include "widget/tip.h"
#include "widget/soundvolume.h"

static const char *sPropertyFavourite         = "fav";
static const char *sPropertyPlayStatus        = "playstatus";

static const QString sPlayStatusValuePlaying    = "playing";
static const QString sPlayStatusValuePause      = "pause";
static const QString sPlayStatusValueStop       = "stop";

static const QString sDefaultCover = ":/image/cover_welcome.png";

class FooterPrivate
{
public:
    FooterPrivate(Footer *parent) : q_ptr(parent)
    {
        hintFilter = new HintFilter;
    }

    void updateQssProperty(QWidget *w, const char *name, const QVariant &value);
    void installTipHint(QWidget *w, const QString &hintstr);
    void installHint(QWidget *w, QWidget *hint);
    void initConnection();

    Cover           *cover      = nullptr;
    Label  *title      = nullptr;
    Label  *artist    = nullptr;
    QPushButton     *btPlay     = nullptr;
    QPushButton     *btPrev     = nullptr;
    QPushButton     *btNext     = nullptr;
    QPushButton     *btFavorite = nullptr;
    QPushButton     *btLyric    = nullptr;
    QPushButton     *btPlayList = nullptr;
    ModeButton      *btPlayMode = nullptr;
    QPushButton     *btSound    = nullptr;
    Slider          *progress   = nullptr;
    HintFilter      *hintFilter = nullptr;
    SoundVolume     *volSlider  = nullptr;

    PlaylistPtr     m_playinglist;
    MusicMeta       m_playingMeta;
    int             m_mode;

    Footer *q_ptr;
    Q_DECLARE_PUBLIC(Footer);
};

void FooterPrivate::updateQssProperty(QWidget *w, const char *name, const QVariant &value)
{
    Q_Q(Footer);
    w->setProperty(name, value);
    q->style()->unpolish(w);
    q->style()->polish(w);
    w->repaint();
}

void FooterPrivate::installTipHint(QWidget *w, const QString &hintstr)
{
    Q_Q(Footer);
    auto hintWidget = new Tip(QPixmap(), hintstr, q);
    hintWidget->setFixedHeight(32);
    installHint(w, hintWidget);
}

void FooterPrivate::installHint(QWidget *w, QWidget *hint)
{
    w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hint));
    w->installEventFilter(hintFilter);
}

void FooterPrivate::initConnection()
{
    Q_Q(Footer);

    q->connect(btPlayMode, &ModeButton::modeChanged,
               q, &Footer::modeChanged);

    q->connect(progress, &Slider::valueAccpet, q, [ = ](int value) {
        auto range = progress->maximum() - progress->minimum();
        Q_ASSERT(range != 0);
        emit q->changeProgress(value, range);
    });

    q->connect(btPlay, &QPushButton::released, q, [ = ]() {
        auto status = btPlay->property(sPropertyPlayStatus).toString();
        if (status == sPlayStatusValuePlaying) {
            emit q->pause(m_playinglist, m_playingMeta);
        } else  if (status == sPlayStatusValuePause) {
            emit q->resume(m_playinglist, m_playingMeta);
        } else {
            emit q->play(m_playinglist, m_playingMeta);
        }
//        if (!d->m_playinglist) {
//            emit play(d->m_playinglist, d->m_playingMeta);
//            return;
//        }

//        if (d->m_playinglist && 0 == d->m_playinglist->length()) {
//            emit play(d->m_playinglist, d->m_playingMeta);
//            return;
//        }


//        if (status == sPlayStatusValueStop) {
//            emit play(d->m_playinglist, d->m_playingMeta);
//        }
//        if (status == sPlayStatusValuePause) {
//            emit resume(d->m_playinglist, d->m_playingMeta);
//        }
    });

    q->connect(btPrev, &QPushButton::released, q, [ = ]() {
        emit q->prev(m_playinglist, m_playingMeta);
    });
    q->connect(btNext, &QPushButton::released, q, [ = ]() {
        emit q->next(m_playinglist, m_playingMeta);
    });

    q->connect(btFavorite, &QPushButton::released, q, [ = ]() {
        qDebug() << "btFavorite---------------------------";
        emit q->toggleFavourite(m_playingMeta);
    });
    q->connect(title, &Label::clicked, q, [ = ](bool) {
        emit q->locateMusic(m_playinglist, m_playingMeta);
    });
    q->connect(cover, &Label::clicked, q, [ = ](bool) {
        emit  q->toggleLyricView();
    });
    q->connect(btLyric, &QPushButton::released, q, [ = ]() {
        emit  q->toggleLyricView();
    });
    q->connect(btPlayList, &QPushButton::released, q, [ = ]() {
        emit q->togglePlaylist();
    });
    q->connect(btSound, &QPushButton::released, q, [ = ]() {
        emit q->toggleMute();
    });
    q->connect(volSlider, &SoundVolume::volumeChanged,
               q, &Footer::volumeChanged);
}

Footer::Footer(QWidget *parent) :
    QFrame(parent), d_ptr(new FooterPrivate(this))
{
    Q_D(Footer);

    setFocusPolicy(Qt::ClickFocus);
    setObjectName("Footer");

    auto vboxlayout = new QVBoxLayout(this);
    vboxlayout->setSpacing(0);
    vboxlayout->setMargin(0);

    auto hoverFilter = new HoverFilter(this);

    d->progress = new Slider(Qt::Horizontal);
    d->progress->setObjectName("FooterProgress");
    d->progress->setFixedHeight(12);
    d->progress->setMinimum(0);
    d->progress->setMaximum(1000);
    d->progress->setValue(0);

    auto layout = new QHBoxLayout();
    layout->setContentsMargins(10, 0, 20, 10);
    layout->setSpacing(20);

    d->cover = new Cover;
    d->cover->setObjectName("FooterCover");
    d->cover->setFixedSize(40, 40);
    d->cover->setRadius(0);
    d->cover->setCoverPixmap(QPixmap(sDefaultCover));
    d->cover->installEventFilter(hoverFilter);

    d->title = new Label;
    d->title->setObjectName("FooterTitle");
    d->title->setMaximumWidth(240);
    d->title->setText(tr("Unknow Title"));
    d->title->installEventFilter(hoverFilter);

    d->artist = new Label;
    d->artist->setObjectName("FooterArtist");
    d->artist->setMaximumWidth(240);
    d->artist->setText(tr("Unknow Artist"));

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
    d->btSound->setProperty("volume", "mid");

    d->btPlayList = new QPushButton;
    d->btPlayList->setObjectName("FooterActionPlayList");
    d->btPlayList->setFixedSize(24, 24);


    d->btSound->installEventFilter(this);
    d->installTipHint(d->btFavorite, tr("Add to my favorites"));
    d->installTipHint(d->btLyric, tr("Lyric"));
    d->installTipHint(d->btPlayMode, tr("Play Mode"));
    d->installTipHint(d->btPlayList, tr("Playlist"));
    d->volSlider = new SoundVolume(this);
    d->volSlider->setProperty("DelayHide", true);
    d->volSlider->setProperty("NoDelayShow", true);
    d->installHint(d->btSound, d->volSlider);

    auto infoWidget = new QWidget;
    auto infoLayout = new QHBoxLayout(infoWidget);
    auto musicMetaLayout = new QVBoxLayout;
    musicMetaLayout->addWidget(d->title);
    musicMetaLayout->addWidget(d->artist);
    musicMetaLayout->setSpacing(0);
    infoLayout->setMargin(0);
    infoLayout->setSpacing(0);
    infoLayout->addWidget(d->cover, 0, Qt::AlignLeft | Qt::AlignVCenter);
    infoLayout->addSpacing(10);
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
    actLayout->addWidget(d->btPlayList, 0, Qt::AlignRight | Qt::AlignVCenter);

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
    vboxlayout->addLayout(layout);

    d->title->hide();
    d->artist->hide();
    d->btPrev->hide();
    d->btNext->hide();
    d->btFavorite->hide();
    d->btLyric->hide();

    ThemeManager::instance()->regisetrWidget(this);

    d->initConnection();

    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, sPlayStatusValueStop);
    d->updateQssProperty(this, sPropertyPlayStatus, sPlayStatusValueStop);
}

Footer::~Footer()
{

}

void Footer::enableControl(bool enable)
{
    Q_D(Footer);

    d->btPrev->setEnabled(enable);
    d->btNext->setEnabled(enable);
    d->btFavorite->setEnabled(enable);
    d->btLyric->setEnabled(enable);
    d->btPlayList->setEnabled(enable);
    d->btPlayMode->setEnabled(enable);
    d->btSound->setEnabled(enable);
    d->progress->setEnabled(enable);

    d->cover->blockSignals(!enable);
    d->title->blockSignals(!enable);
    d->artist->blockSignals(!enable);
}

void Footer::initData(PlaylistPtr current, int mode)
{
    Q_D(Footer);
    d->m_mode = mode;
    d->m_playinglist = current;
    d->btPlayMode->setMode(mode);
}

void Footer::mouseMoveEvent(QMouseEvent *event)
{
    // TODO: ingore sub control
    Qt::MouseButton button = event->buttons() & Qt::LeftButton ? Qt::LeftButton : Qt::NoButton;
    if (event->buttons() == Qt::LeftButton /*&& d->mousePressed*/) {
        emit mouseMoving(button);
    }
}

bool Footer::eventFilter(QObject *obj, QEvent *event)
{
    Q_D(Footer);
    switch (event->type()) {
    case QEvent::Wheel: {
        qDebug() << obj;
        if (obj != d->btSound) {
            return QObject::eventFilter(obj, event);
        }
        auto we = dynamic_cast<QWheelEvent *>(event);
        if (we->angleDelta().y() > 0) {
            d->volSlider->onVolumeChanged(d->volSlider->volume() + 5);
            emit this->volumeChanged(d->volSlider->volume());
        } else {
            d->volSlider->onVolumeChanged(d->volSlider->volume() - 5);
            emit this->volumeChanged(d->volSlider->volume());
        }
        return true;
    }
    default:
        return QObject::eventFilter(obj, event);
    }
}

void Footer::onMusicAdded(PlaylistPtr playlist, const MusicMeta &info)
{
    Q_D(Footer);
    if (playlist->id() == FavMusicListID)
        if (info.hash == d->m_playingMeta.hash) {
            d->updateQssProperty(d->btFavorite, sPropertyFavourite, true);
        }
}

void Footer::onMusicListAdded(PlaylistPtr playlist, const MusicMetaList &infolist)
{
    Q_D(Footer);
    if (playlist->id() == FavMusicListID)
        for (auto &meta : infolist) {
            if (meta.hash == d->m_playingMeta.hash) {
                d->updateQssProperty(d->btFavorite, sPropertyFavourite, true);
            }
        }
}

void Footer::onMusicRemoved(PlaylistPtr playlist, const MusicMeta &info)
{
    Q_D(Footer);
    if (playlist->id() == FavMusicListID)
        if (info.hash == d->m_playingMeta.hash) {
            d->updateQssProperty(d->btFavorite, sPropertyFavourite, false);
        }
}

void Footer::onMusicPlayed(PlaylistPtr playlist, const MusicMeta &info)
{
    Q_D(Footer);
    d->title->setText(info.title);

    if (!info.artist.isEmpty()) {
        d->artist->setText(info.artist);
    } else {
        d->artist->setText(tr("Unknow Artist"));
    }

    this->enableControl(true);
    d->title->show();
    d->artist->show();
    d->btPrev->show();
    d->btNext->show();
    d->btFavorite->show();
    d->btLyric->show();

    d->m_playinglist = playlist;
    d->m_playingMeta = info;

    d->updateQssProperty(d->btFavorite, sPropertyFavourite, info.favourite);
    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, sPlayStatusValuePlaying);
    d->updateQssProperty(this, sPropertyPlayStatus, sPlayStatusValuePlaying);
}

void Footer::onMusicPause(PlaylistPtr playlist, const MusicMeta &meta)
{
    Q_D(Footer);
    qDebug() << meta.title << "pause";
    if (meta.hash != d->m_playingMeta.hash || playlist != d->m_playinglist) {
        qWarning() << "can not pasue" << d->m_playinglist << playlist
                   << d->m_playingMeta.hash << meta.hash;
        return;
    }
    auto status = sPlayStatusValuePause;
    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, status);
}

void Footer::onMusicStoped(PlaylistPtr playlist, const MusicMeta &meta)
{
    Q_D(Footer);

    onProgressChanged(0, 1);
    this->enableControl(false);
    d->title->hide();
    d->artist->hide();
    d->cover->setCoverPixmap(QPixmap(sDefaultCover));
    d->cover->repaint();
    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, sPlayStatusValueStop);
    d->updateQssProperty(this, sPropertyPlayStatus, sPlayStatusValueStop);
}

void Footer::onProgressChanged(qint64 value, qint64 duration)
{
    Q_D(Footer);
    auto length = d->progress->maximum() - d->progress->minimum();
    Q_ASSERT(length != 0);

    auto progress = 0;
    if (0 != duration) {
        progress = static_cast<int>(length * value / duration);
    }

    if (d->progress->signalsBlocked()) {
        return;
    }

    d->progress->blockSignals(true);
    d->progress->setValue(progress);
    d->progress->blockSignals(false);
}

void Footer::onCoverChanged(const MusicMeta &info, const QByteArray &coverData)
{
    Q_D(Footer);
    if (info.hash != d->m_playingMeta.hash) {
        return;
    }

    QPixmap coverPixmap = coverData.length() > 1024 ?
                          QPixmap::fromImage(QImage::fromData(coverData)) :
                          QPixmap(sDefaultCover);

    d->cover->setCoverPixmap(coverPixmap);
    d->cover->repaint();
}

void Footer::onVolumeChanged(int volume)
{
    Q_D(Footer);
    QString status = "mid";
    if (volume > 77) {
        status = "high";
    } else if (volume > 33) {
        status = "mid";
    } else  {
        status = "low";
    }
    d->updateQssProperty(d->btSound, "volume", status);
    d->volSlider->onVolumeChanged(volume);
}

void Footer::onMutedChanged(bool muted)
{
    Q_D(Footer);
    qDebug() << muted;
    if (muted) {
        d->updateQssProperty(d->btSound, "volume", "mute");
        d->volSlider->onVolumeChanged(0);
    }
}

