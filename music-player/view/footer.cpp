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
#include <QStackedLayout>

#include <thememanager.h>

#include "../musicapp.h"
#include "../core/playlistmanager.h"
#include "../core/player.h"

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
    Label           *title      = nullptr;
    Label           *artist     = nullptr;
    QPushButton     *btPlay     = nullptr;
    QPushButton     *btPrev     = nullptr;
    QPushButton     *btNext     = nullptr;
    QPushButton     *btFavorite = nullptr;
    QPushButton     *btLyric    = nullptr;
    QPushButton     *btPlayList = nullptr;
    ModeButton      *btPlayMode = nullptr;
    QPushButton     *btSound    = nullptr;
    Slider          *progress   = nullptr;
    SoundVolume     *volSlider  = nullptr;

    HintFilter          *hintFilter         = nullptr;
    HoverShadowFilter   *hoverShadowFilter  = nullptr;

    QFrame          *ctlWidget  = nullptr;

    PlaylistPtr     m_playinglist;
    MetaPtr         m_playingMeta;

    QString         defaultCover = "";

    int             m_mode;
    bool            enableMove = false;

    Footer *q_ptr;
    Q_DECLARE_PUBLIC(Footer)
};

void FooterPrivate::updateQssProperty(QWidget *w, const char *name, const QVariant &value)
{
    Q_Q(Footer);
    w->setProperty(name, value);
    q->style()->unpolish(w);
    q->style()->polish(w);
    w->update();
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
    q, [ = ](int mode) {
        emit q->modeChanged(mode);

        auto hintWidget = btPlayMode->property("HintWidget").value<Tip *>();
        hintFilter->showHitsFor(btPlayMode, hintWidget);
    });

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
    q->connect(btLyric, &QPushButton::released, q, [ = ]() {
        emit  q->toggleLyricView();
    });
    q->connect(btPlayList, &QPushButton::released, q, [ = ]() {
        emit q->togglePlaylist();
    });
    q->connect(btSound, &QPushButton::pressed, q, [ = ]() {
        emit q->toggleMute();
    });
    q->connect(volSlider, &SoundVolume::volumeChanged, q, [ = ](int vol) {
        q->onVolumeChanged(vol);
        emit q->volumeChanged(vol);
    });

    q->connect(q, &Footer::mouseMoving, q, [ = ](Qt::MouseButton) {
        hintFilter->hideAll();
    });
}

Footer::Footer(QWidget *parent) :
    QFrame(parent), d_ptr(new FooterPrivate(this))
{
    Q_D(Footer);

    setFocusPolicy(Qt::ClickFocus);
    setObjectName("Footer");

    auto vboxlayout = new QVBoxLayout(this);
    vboxlayout->setSpacing(0);
    vboxlayout->setContentsMargins(0, 0, 0, 2);

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

    auto stackedLayout = new QStackedLayout;
    stackedLayout->setStackingMode(QStackedLayout::StackAll);

    d->cover = new Cover;
    d->cover->setObjectName("FooterCover");
    d->cover->setFixedSize(40, 40);
    d->cover->setRadius(0);

    auto coverHoverBt = new QPushButton();
    coverHoverBt->setObjectName("FooterCoverHover");
    coverHoverBt->setFixedSize(40, 40);

    stackedLayout->addWidget(d->cover);
    stackedLayout->addWidget(coverHoverBt);
    coverHoverBt->installEventFilter(hoverFilter);

    d->title = new Label;
    d->title->setObjectName("FooterTitle");
    d->title->setMaximumWidth(240);
    d->title->setText(tr("Unknow Title"));
//    d->title->installEventFilter(hoverFilter);

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

    d->hoverShadowFilter = new HoverShadowFilter;
    d->title->installEventFilter(d->hoverShadowFilter);

    d->btSound->installEventFilter(this);
    d->installTipHint(d->btPrev, tr("Previous"));
    d->installTipHint(d->btNext, tr("Next"));
    d->installTipHint(d->btPlay, tr("Play/Pause"));
    d->installTipHint(d->btFavorite, tr("Add to my favorites"));
    d->installTipHint(d->btLyric, tr("Lyric"));
    d->installTipHint(d->btPlayMode, tr("Play Mode"));
    d->installTipHint(d->btPlayList, tr("Playlist"));
    d->volSlider = new SoundVolume(this);
    d->volSlider->setProperty("DelayHide", true);
    d->volSlider->setProperty("NoDelayShow", true);
    d->installHint(d->btSound, d->volSlider);

    auto metaWidget = new QFrame;
    auto metaLayout = new QHBoxLayout(metaWidget);
    auto musicMetaLayout = new QVBoxLayout;
    metaLayout->setMargin(0);
    metaLayout->setSpacing(0);

    musicMetaLayout->addWidget(d->title);
    musicMetaLayout->addWidget(d->artist);
    musicMetaLayout->setSpacing(0);
    metaLayout->addLayout(stackedLayout);
    metaLayout->addSpacing(10);
    metaLayout->addLayout(musicMetaLayout, 0);
    metaLayout->addStretch();

    d->ctlWidget = new QFrame(this);
//    d->ctlWidget->setStyleSheet("border: 1px solid red;");
    auto ctlLayout = new QHBoxLayout(d->ctlWidget);
    ctlLayout->setMargin(0);
    ctlLayout->setSpacing(30);
    ctlLayout->addWidget(d->btPrev, 0, Qt::AlignCenter);
    ctlLayout->addWidget(d->btPlay, 0, Qt::AlignCenter);
    ctlLayout->addWidget(d->btNext, 0, Qt::AlignCenter);
    d->ctlWidget->adjustSize();

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
    metaWidget->setSizePolicy(sp);
    actWidget->setSizePolicy(sp);

    layout->addWidget(metaWidget, 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addStretch();
//    layout->addWidget(d->ctlWidget, 0, Qt::AlignCenter);
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

    d->btPrev->setFocusPolicy(Qt::NoFocus);
    d->btNext->setFocusPolicy(Qt::NoFocus);
    d->btFavorite->setFocusPolicy(Qt::NoFocus);
    d->btLyric->setFocusPolicy(Qt::NoFocus);
    d->btPlayMode ->setFocusPolicy(Qt::NoFocus);
    d->btSound->setFocusPolicy(Qt::NoFocus);
    d->btPlayList->setFocusPolicy(Qt::NoFocus);

    ThemeManager::instance()->regisetrWidget(this);

    d->initConnection();

    connect(coverHoverBt, &QPushButton::clicked, this, [ = ](bool) {
        emit toggleLyricView();
    });

    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, sPlayStatusValueStop);
    d->updateQssProperty(this, sPropertyPlayStatus, sPlayStatusValueStop);
    d->cover->setCoverPixmap(QPixmap(d->defaultCover));
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

QString Footer::defaultCover() const
{
    Q_D(const Footer);
    return d->defaultCover;
}


void Footer::mousePressEvent(QMouseEvent *event)
{
    Q_D(Footer);
    QFrame::mousePressEvent(event);
    auto subCtlPos = d->progress->mapFromParent(event->pos());
    if (d->progress->rect().contains(subCtlPos)
            || !this->rect().contains(event->pos())) {
        d->enableMove = false;
        return;
    }
    d->enableMove = true;
}

void Footer::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(Footer);
    QFrame::mouseReleaseEvent(event);
    d->enableMove = false;
}

void Footer::mouseMoveEvent(QMouseEvent *event)
{
    // TODO: ingore sub control
    Q_D(Footer);
    QFrame::mouseMoveEvent(event);

    Qt::MouseButton button = event->buttons() & Qt::LeftButton ? Qt::LeftButton : Qt::NoButton;
    if (d->enableMove && event->buttons() == Qt::LeftButton /*&& d->mousePressed*/) {
        emit mouseMoving(button);
    }
}

bool Footer::eventFilter(QObject *obj, QEvent *event)
{
    Q_D(Footer);
    switch (event->type()) {
    case QEvent::Wheel: {
        if (obj != d->btSound) {
            return QObject::eventFilter(obj, event);
        }
        auto we = dynamic_cast<QWheelEvent *>(event);
        if (we->angleDelta().y() > 0) {
            auto vol = d->volSlider->volume() + Player::VolumeStep;
            if (vol > 100) {
                vol = 100;
            }
            onVolumeChanged(vol);
            emit this->volumeChanged(d->volSlider->volume());
        } else {
            auto vol = d->volSlider->volume() - Player::VolumeStep;
            if (vol < 0) {
                vol = 0;
            }
            onVolumeChanged(vol);
            emit this->volumeChanged(d->volSlider->volume());
        }
        return true;
    }
    default:
        return QObject::eventFilter(obj, event);
    }
}

void Footer::onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_D(Footer);
    if (playlist->id() == FavMusicListID)
        for (auto &meta : metalist) {
            if (meta->hash == d->m_playingMeta->hash) {
                d->updateQssProperty(d->btFavorite, sPropertyFavourite, true);
            }
        }
}

void Footer::onMusicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_D(Footer);
    if (playlist->id() == FavMusicListID)
        for (auto &meta : metalist) {
            if (meta == d->m_playingMeta) {
                d->updateQssProperty(d->btFavorite, sPropertyFavourite, false);
            }
        }
}

void Footer::onMusicPlayed(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Footer);

    qDebug() << "FFFFFFFFFFFFFFF"
             << playlist->displayName();

    QFontMetrics fm(d->title->font());
    auto text = fm.elidedText(meta->title, Qt::ElideMiddle, d->title->maximumWidth());
    d->title->setText(text);

    if (!meta->artist.isEmpty()) {
        d->artist->setText(meta->artist);
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
    d->m_playingMeta = meta;

    d->updateQssProperty(d->btFavorite, sPropertyFavourite, meta->favourite);
    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, sPlayStatusValuePlaying);
    d->updateQssProperty(this, sPropertyPlayStatus, sPlayStatusValuePlaying);
}

void Footer::onMusicPause(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Footer);
    qDebug() << meta->title << "pause";
    if (meta->hash != d->m_playingMeta->hash || playlist != d->m_playinglist) {
        qWarning() << "can not pasue" << d->m_playinglist << playlist
                   << d->m_playingMeta->hash << meta->hash;
        return;
    }
    auto status = sPlayStatusValuePause;
    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, status);
}

void Footer::onMusicStoped(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Footer);

    Q_UNUSED(playlist);
    Q_UNUSED(meta);

    onProgressChanged(0, 1);
    this->enableControl(false);
    d->title->hide();
    d->artist->hide();
    d->cover->setCoverPixmap(QPixmap(d->defaultCover));
    d->cover->update();
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

void Footer::onCoverChanged(const MetaPtr meta, const DMusic::SearchMeta &song, const QByteArray &coverData)
{
    Q_D(Footer);
    if (meta != d->m_playingMeta) {
        return;
    }

    QPixmap coverPixmap = coverData.length() > 1024 ?
                          QPixmap::fromImage(QImage::fromData(coverData)) :
                          QPixmap(d->defaultCover);

    d->cover->setCoverPixmap(coverPixmap);
    d->cover->update();
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

//    qDebug() << status << volume;
    d->volSlider->onVolumeChanged(volume);
}

void Footer::onMutedChanged(bool muted)
{
    Q_D(Footer);
//    qDebug() << muted;
    if (muted) {
        d->updateQssProperty(d->btSound, "volume", "mute");
        d->volSlider->onVolumeChanged(0);
    }
}

void Footer::onModeChange(int mode)
{
    Q_D(Footer);

    if (d->m_mode == mode) {
        return;
    }
    d->btPlayMode->blockSignals(true);
    d->btPlayMode->setMode(mode);
    d->btPlayMode->blockSignals(false);
    d->m_mode = mode;

    auto hintWidget = d->btPlayMode->property("HintWidget").value<Tip *>();
    QString playmode;
    switch (mode) {
    case 0:
        playmode = Footer::tr("Repeat All");
        break;
    case 1:
        playmode = Footer::tr("Repeat Single");
        break;
    case 2:
        playmode = Footer::tr("Shuffle");
        break;
    }
    if (hintWidget) {
        hintWidget->setText(playmode);
    }
}

void Footer::onUpdateMetaCodec(const MetaPtr meta)
{
    Q_D(Footer);
    if (d->m_playingMeta->hash == meta->hash) {
        d->title->setText(meta->title);
        if (!meta->artist.isEmpty()) {
            d->artist->setText(meta->artist);
        } else {
            d->artist->setText(tr("Unknow Artist"));
        }
    }
}

void Footer::setDefaultCover(QString defaultCover)
{
    Q_D(Footer);
    d->defaultCover = defaultCover;
}

void Footer::resizeEvent(QResizeEvent *event)
{
    Q_D(Footer);
    QFrame::resizeEvent(event);

    auto center = this->rect().center() - d->ctlWidget->rect().center();

    d->ctlWidget->move(center);
    d->ctlWidget->raise();
}


