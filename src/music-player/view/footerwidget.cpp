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

#include "footerwidget.h"

#include <QDebug>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStackedLayout>

#include <DHiDPIHelper>
#include <DToast>
#include <DPushButton>
#include <DProgressBar>

#include <metadetector.h>

#include "../musicapp.h"
#include "../core/playlistmanager.h"
#include "../core/player.h"
#include "../core/metasearchservice.h"

#include "widget/filter.h"
#include "widget/slider.h"
#include "widget/modebuttom.h"
#include "widget/label.h"
#include "widget/cover.h"
#include "widget/soundvolume.h"
#include "widget/musicimagebutton.h"
#include "widget/musicpixmapbutton.h"
#include "widget/waveform.h"

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

    Label           *title      = nullptr;
    Label           *artist     = nullptr;

    MusicPixmapButton *btCover    = nullptr;
    MusicImageButton  *btPlay     = nullptr;
    MusicImageButton  *btPrev     = nullptr;
    MusicImageButton  *btNext     = nullptr;
    MusicImageButton  *btFavorite = nullptr;
    MusicImageButton  *btLyric    = nullptr;
    MusicImageButton  *btPlayList = nullptr;
    ModeButton        *btPlayMode = nullptr;
    MusicImageButton  *btSound    = nullptr;
    Slider            *progress   = nullptr;
    SoundVolume       *volSlider  = nullptr;
    DFrame            *ctlWidget  = nullptr;
    Waveform          *waveform   = nullptr;

    HintFilter          *hintFilter         = nullptr;
    HoverShadowFilter   *hoverShadowFilter  = nullptr;

    PlaylistPtr     activingPlaylist;
    MetaPtr         activingMeta;

    QString         defaultCover    = ":/common/image/info_cover.svg";

    int             mode            = -1;
    bool            enableMove      = false;

    Footer *q_ptr;
    Q_DECLARE_PUBLIC(Footer)
};

void FooterPrivate::updateQssProperty(QWidget *w, const char *name, const QVariant &value)
{
    Q_Q(Footer);
    w->setProperty(name, value);
    w->update();
}

void FooterPrivate::installTipHint(QWidget *w, const QString &hintstr)
{
    Q_Q(Footer);
    // TODO: parent must be mainframe
    auto hintWidget = new Dtk::Widget::DToast(q->parentWidget());
    hintWidget->layout()->setContentsMargins(10, 0, 10, 0);
    hintWidget->setText(hintstr);
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
        Q_EMIT q->modeChanged(mode);
        auto hintWidget = btPlayMode->property("HintWidget").value<Dtk::Widget::DToast *>();
        hintFilter->showHitsFor(btPlayMode, hintWidget);
    });

    q->connect(progress, &Slider::valueAccpet, q, [ = ](int value) {
        auto range = progress->maximum() - progress->minimum();
        Q_ASSERT(range != 0);
        Q_EMIT q->changeProgress(value, range);
    });
    q->connect(progress, &Slider::realHeightChanged, q, [ = ](qreal value) {
        Q_EMIT q->progressRealHeightChanged(value);
    });

    q->connect(btPlay, &DPushButton::released, q, [ = ]() {
        auto status = btPlay->property(sPropertyPlayStatus).toString();
        if (status == sPlayStatusValuePlaying) {
            Q_EMIT q->pause(activingPlaylist, activingMeta);
        } else  if (status == sPlayStatusValuePause) {
            Q_EMIT q->resume(activingPlaylist, activingMeta);
        } else {
            Q_EMIT q->play(activingPlaylist, activingMeta);
        }
    });

    q->connect(btPrev, &DPushButton::released, q, [ = ]() {
        Q_EMIT q->prev(activingPlaylist, activingMeta);
    });
    q->connect(btNext, &DPushButton::released, q, [ = ]() {
        Q_EMIT q->next(activingPlaylist, activingMeta);
    });

    q->connect(btFavorite, &DPushButton::released, q, [ = ]() {
        Q_EMIT q->toggleFavourite(activingMeta);
    });
    q->connect(title, &Label::clicked, q, [ = ](bool) {
        Q_EMIT q->locateMusic(activingPlaylist, activingMeta);
    });
    q->connect(btLyric, &DPushButton::released, q, [ = ]() {
        Q_EMIT  q->toggleLyricView();
    });
    q->connect(btPlayList, &DPushButton::released, q, [ = ]() {
        Q_EMIT q->togglePlaylist();
    });
    q->connect(btSound, &DPushButton::pressed, q, [ = ]() {
        Q_EMIT q->toggleMute();
    });
    q->connect(volSlider, &SoundVolume::volumeChanged, q, [ = ](int vol) {
        q->onVolumeChanged(vol);
        Q_EMIT q->volumeChanged(vol);
    });

    q->connect(q, &Footer::mouseMoving, q, [ = ](Qt::MouseButton) {
        hintFilter->hideAll();
    });

    q->connect(q, &Footer::audioBufferProbed, waveform, &Waveform::onAudioBufferProbed);
}

Footer::Footer(QWidget *parent) :
    DBlurEffectWidget(parent), d_ptr(new FooterPrivate(this))
{
    Q_D(Footer);

    setFocusPolicy(Qt::ClickFocus);
    setObjectName("Footer");
    setMode(DBlurEffectWidget::GaussianBlur);
    setMaskColor(QColor(0, 0, 0, 75));

    auto mainVBoxlayout = new QVBoxLayout(this);
    mainVBoxlayout->setSpacing(0);
    mainVBoxlayout->setContentsMargins(0, 0, 0, 0);

    auto hoverFilter = new HoverFilter(this);

    d->progress = new Slider(Qt::Horizontal);
    d->progress->setObjectName("FooterProgress");
    d->progress->setFixedHeight(6);
    d->progress->setMinimum(0);
    d->progress->setMaximum(1000);
    d->progress->setValue(0);

    auto layout = new QHBoxLayout();
    layout->setContentsMargins(10, 0, 20, 0);
    layout->setSpacing(20);

    d->btCover = new MusicPixmapButton();
    d->btCover->setObjectName("FooterCoverHover");
    d->btCover->setFixedSize(40, 40);

    d->title = new Label;
    d->title->setObjectName("FooterTitle");
    d->title->setMaximumWidth(240);
    d->title->setText(tr("Unknown Title"));
//    d->title->installEventFilter(hoverFilter);

    d->artist = new Label;
    d->artist->setObjectName("FooterArtist");
    d->artist->setMaximumWidth(240);
    d->artist->setText(tr("Unknown artist"));

    d->btPlay = new MusicImageButton(":/mpimage/normal/play_normal.svg",
                                     ":/mpimage/hover/play_hover.svg",
                                     ":/mpimage/press/play_press.svg");
    d->btPlay->setPropertyPic(sPropertyPlayStatus, sPlayStatusValuePlaying,
                              ":/mpimage/normal/suspend_normal.svg",
                              ":/mpimage/hover/suspend_hover.svg",
                              ":/mpimage/press/suspend_press.svg");
    d->btPlay->setObjectName("FooterActionPlay");
    d->btPlay->setFixedSize(36, 36);

    d->btPrev = new MusicImageButton(":/mpimage/normal/last_normal.svg",
                                     ":/mpimage/hover/last_hover.svg",
                                     ":/mpimage/press/last_press.svg");
    d->btPrev->setObjectName("FooterActionPrev");
    d->btPrev->setFixedSize(36, 36);

    d->btNext = new MusicImageButton(":/mpimage/normal/next_normal.svg",
                                     ":/mpimage/hover/next_hover.svg",
                                     ":/mpimage/press/next_press.svg");
    d->btNext->setObjectName("FooterActionNext");
    d->btNext->setFixedSize(36, 36);

    d->btFavorite = new MusicImageButton(":/mpimage/normal/collection_normal.svg",
                                         ":/mpimage/hover/collection_hover.svg",
                                         ":/mpimage/press/collection_press.svg");
    d->btFavorite->setPropertyPic(sPropertyFavourite, QVariant(true),
                                  ":/mpimage/normal/my_collection_normal.svg",
                                  ":/mpimage/hover/my_collection_hover.svg",
                                  ":/mpimage/press/my_collection_press.svg");
    d->btFavorite->setObjectName("FooterActionFavorite");
    d->btFavorite->setFixedSize(30, 30);

    d->btLyric = new MusicImageButton(":/mpimage/normal/lyric_normal.svg",
                                      ":/mpimage/hover/lyric_hover.svg",
                                      ":/mpimage/press/lyric_press.svg");
    d->btLyric->setObjectName("FooterActionLyric");
    d->btLyric->setFixedSize(36, 36);

    QStringList modes;
    modes << ":/mpimage/normal/sequential_loop_normal.svg"
          << ":/mpimage/normal/single_tune_circulation_normal.svg"
          << ":/mpimage/normal/cross_cycling_normal.svg";
    d->btPlayMode = new ModeButton;
    d->btPlayMode->setObjectName("FooterActionPlayMode");
    d->btPlayMode->setFixedSize(36, 36);
    d->btPlayMode->setModeIcons(modes);

    d->btSound = new MusicImageButton(":/mpimage/normal/volume_normal.svg",
                                      ":/mpimage/hover/volume_hover.svg",
                                      ":/mpimage/press/volume_press.svg");
    d->btSound->setPropertyPic("btSound", QVariant("mid"),
                               ":/mpimage/normal/volume_lessen_normal.svg",
                               ":/mpimage/hover/volume_lessen_hover.svg",
                               ":/mpimage/press/volume_lessen_press.svg");
    d->btSound->setPropertyPic("btSound", QVariant("low"),
                               ":/mpimage/normal/volume_add_normal.svg",
                               ":/mpimage/hover/volume_add_hover.svg",
                               ":/mpimage/press/volume_add_press.svg");
    d->btSound->setPropertyPic("btSound", QVariant("mute"),
                               ":/mpimage/normal/mute_normal.svg",
                               ":/mpimage/hover/mute_hover.svg",
                               ":/mpimage/press/mute_press.svg");
    d->btSound->setObjectName("FooterActionSound");
    d->btSound->setFixedSize(36, 36);
    d->btSound->setProperty("volume", "mid");

    d->btPlayList = new MusicImageButton(":/mpimage/normal/playlist_normal.svg",
                                         ":/mpimage/hover/playlist_hover.svg",
                                         ":/mpimage/press/playlist_press.svg");
    d->btPlayList->setObjectName("FooterActionPlayList");
    d->btPlayList->setFixedSize(36, 36);
//    d->btPlayList->hide();

    d->hoverShadowFilter = new HoverShadowFilter;
    d->title->installEventFilter(d->hoverShadowFilter);

    d->btSound->installEventFilter(this);
    d->installTipHint(d->btPrev, tr("Previous"));
    d->installTipHint(d->btNext, tr("Next"));
    d->installTipHint(d->btPlay, tr("Play/Pause"));
    d->installTipHint(d->btFavorite, tr("Favorite"));
    d->installTipHint(d->btLyric, tr("Lyrics"));
    d->installTipHint(d->btPlayMode, tr("Play Mode"));
    d->installTipHint(d->btPlayList, tr("Playlist"));
    d->volSlider = new SoundVolume(this->parentWidget());
    d->volSlider->hide();
    d->volSlider->setProperty("DelayHide", true);
    d->volSlider->setProperty("NoDelayShow", true);
    d->installHint(d->btSound, d->volSlider);

    auto musicMetaLayout = new QVBoxLayout;
    musicMetaLayout->setContentsMargins(0, 0, 0, 0);
    musicMetaLayout->setSpacing(0);
    musicMetaLayout->addWidget(d->title);
    musicMetaLayout->addWidget(d->artist);

    auto metaWidget = new DFrame;
//    metaWidget->setStyleSheet("border: 1px solid red;");
    auto metaLayout = new QHBoxLayout(metaWidget);
    metaLayout->setContentsMargins(0, 0, 0, 0);
    metaLayout->setSpacing(0);
    metaLayout->addWidget(d->btCover);
    metaLayout->addSpacing(10);
    metaLayout->addLayout(musicMetaLayout, 0);

    d->ctlWidget = new DFrame(this);
//    d->ctlWidget->setStyleSheet("border: 1px solid red;");
    auto ctlLayout = new QHBoxLayout(d->ctlWidget);
    ctlLayout->setMargin(0);
    ctlLayout->setSpacing(30);
    ctlLayout->addWidget(d->btPrev, 0, Qt::AlignCenter);
    ctlLayout->addWidget(d->btPlay, 0, Qt::AlignCenter);
    ctlLayout->addWidget(d->btNext, 0, Qt::AlignCenter);
    d->ctlWidget->adjustSize();

    d->waveform = new Waveform;
    d->waveform->adjustSize();

    auto actWidget = new QWidget;
    auto actLayout = new QHBoxLayout(actWidget);
    actLayout->setMargin(0);
    actLayout->setSpacing(20);
    actLayout->addWidget(d->btFavorite, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btLyric, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btPlayMode, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btSound, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btPlayList, 0, Qt::AlignRight | Qt::AlignVCenter);

//    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
//    sp.setHorizontalStretch(33);
//    metaWidget->setSizePolicy(sp);
//    actWidget->setSizePolicy(sp);

    layout->addWidget(d->ctlWidget);
    layout->addWidget(metaWidget);
    layout->addWidget(d->waveform, 100);
    layout->addWidget(actWidget, 0, Qt::AlignRight | Qt::AlignVCenter);

    mainVBoxlayout->addWidget(d->progress);

    auto controlFrame = new DFrame;
    controlFrame->setObjectName("FooterControlFrame");
    controlFrame->setFixedHeight(60 - d->progress->height() / 2);
    controlFrame->setLayout(layout);

    mainVBoxlayout->addStretch();
    mainVBoxlayout->addWidget(controlFrame);

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

    d->initConnection();

    connect(d->btCover, &DPushButton::clicked, this, [ = ](bool) {
        Q_EMIT toggleLyricView();
        if (d->btCover->property("viewname").toString() != "lyric") {
            d->updateQssProperty(d->btCover, "viewname", "lyric");
        } else {
            d->updateQssProperty(d->btCover, "viewname", "musiclist");
        }
    });

    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, sPlayStatusValueStop);
    d->updateQssProperty(this, sPropertyPlayStatus, sPlayStatusValueStop);
    d->btCover->setIcon(Dtk::Widget::DHiDPIHelper::loadNxPixmap(d->defaultCover));
}

Footer::~Footer()
{

}

int Footer::progressExtentHeight() const
{
    Q_D(const Footer);
    return (d->progress->height()) / 2;
}

void Footer::enableControl(bool enable)
{
    Q_D(Footer);

    d->btCover->setEnabled(enable);
    d->btPrev->setEnabled(enable);
    d->btNext->setEnabled(enable);
    d->btFavorite->setEnabled(enable);
    d->btLyric->setEnabled(enable);
    d->btPlayList->setEnabled(enable);
    d->btPlayMode->setEnabled(enable);
    d->btSound->setEnabled(enable);
    d->progress->setEnabled(enable);

    d->btCover->blockSignals(!enable);
    d->title->blockSignals(!enable);
    d->artist->blockSignals(!enable);
}

void Footer::initData(PlaylistPtr current, int mode)
{
    Q_D(Footer);
    d->mode = mode;
    d->activingPlaylist = current;
    d->btPlayMode->setMode(mode);
}

void Footer::setViewname(const QString &viewname)
{
    Q_D(Footer);
    setProperty("viewname", viewname);
    d->progress->setProperty("viewname", viewname);
}

QString Footer::defaultCover() const
{
    Q_D(const Footer);
    return d->defaultCover;
}

void Footer::mousePressEvent(QMouseEvent *event)
{
    Q_D(Footer);
    DWidget::mousePressEvent(event);
    auto subCtlPos = d->progress->mapFromParent(event->pos());
    if (d->progress->rect().contains(subCtlPos)
            /*|| !this->rect().contains(event->pos())*/) {
        d->enableMove = false;
    } else {
        d->enableMove = true;
    }
}

void Footer::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(Footer);
    DWidget::mouseReleaseEvent(event);
    d->enableMove = false;
}

void Footer::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(Footer);
    Qt::MouseButton button = event->buttons() & Qt::LeftButton ? Qt::LeftButton : Qt::NoButton;
    if (d->enableMove && d->enableMove && event->buttons() == Qt::LeftButton) {
        Q_EMIT mouseMoving(button);
        DWidget::mouseMoveEvent(event);
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
            Q_EMIT this->volumeChanged(d->volSlider->volume());
        } else {
            auto vol = d->volSlider->volume() - Player::VolumeStep;
            if (vol < 0) {
                vol = 0;
            }
            onVolumeChanged(vol);
            Q_EMIT this->volumeChanged(d->volSlider->volume());
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
            if (d->activingMeta && meta == d->activingMeta) {
                d->updateQssProperty(d->btFavorite, sPropertyFavourite, true);
            }
        }
}

void Footer::onMusicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_D(Footer);
    if (playlist->id() == FavMusicListID)
        for (auto &meta : metalist) {
            if (meta == d->activingMeta) {
                d->updateQssProperty(d->btFavorite, sPropertyFavourite, false);
            }
        }
}

void Footer::onMusicPlayed(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Footer);

    QFontMetrics fm(d->title->font());
    auto text = fm.elidedText(meta->title, Qt::ElideMiddle, d->title->maximumWidth());
    d->title->setText(text);

    if (!meta->artist.isEmpty()) {
        d->artist->setText(meta->artist);
    } else {
        d->artist->setText(tr("Unknown artist"));
    }

    QImage cover(d->defaultCover);
    auto coverData = MetaSearchService::coverData(meta);
    if (coverData.length() > 0) {
        cover = QImage::fromData(coverData);
    }
    d->btCover->setIcon(QPixmap::fromImage(cover));
    d->btCover->update();

    this->enableControl(true);
    d->title->show();
    d->artist->show();
    d->btPrev->show();
    d->btNext->show();
    d->btFavorite->show();
    d->btLyric->show();

    d->activingPlaylist = playlist;
    d->activingMeta = meta;

    d->updateQssProperty(d->btFavorite, sPropertyFavourite, meta->favourite);

    if (!meta->invalid) {
        d->updateQssProperty(d->btPlay, sPropertyPlayStatus, sPlayStatusValuePlaying);
        d->updateQssProperty(this, sPropertyPlayStatus, sPlayStatusValuePlaying);
    }
}

void Footer::onMusicError(PlaylistPtr playlist, const MetaPtr meta, int error)
{
    Q_D(Footer);

    if (d->activingMeta && d->activingPlaylist) {
        if (meta != d->activingMeta || playlist != d->activingPlaylist) {
            return;
        }
    }

    if (0 == error) {
        return;
    }

    auto status = sPlayStatusValuePause;
    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, status);
}

void Footer::onMusicPause(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Footer);
    if (meta->hash != d->activingMeta->hash || playlist != d->activingPlaylist) {
        qWarning() << "can not pasue" << d->activingPlaylist << playlist
                   << d->activingMeta->hash << meta->hash;
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
    d->title->hide();
    d->artist->hide();
    d->btFavorite->hide();
    d->activingMeta = MetaPtr();

    d->btCover->setIcon(Dtk::Widget::DHiDPIHelper::loadNxPixmap(d->defaultCover));
    d->btCover->update();
    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, sPlayStatusValueStop);
    d->updateQssProperty(this, sPropertyPlayStatus, sPlayStatusValueStop);
}

void Footer::onMediaLibraryClean()
{
    Q_D(Footer);
    d->btPrev->hide();
    d->btNext->hide();
    d->btFavorite->hide();
    d->btLyric->hide();
    enableControl(false);
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

void Footer::onCoverChanged(const MetaPtr meta, const DMusic::SearchMeta &, const QByteArray &coverData)
{
    Q_D(Footer);
    if (meta != d->activingMeta) {
        return;
    }

    QPixmap coverPixmap = coverData.length() > 1024 ?
                          QPixmap::fromImage(QImage::fromData(coverData)) :
                          Dtk::Widget::DHiDPIHelper::loadNxPixmap(d->defaultCover);
    d->btCover->setIcon(coverPixmap);
    d->btCover->update();
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

    qDebug() << "change play mode to" << mode;
    if (d->mode == mode) {
        return;
    }
    d->btPlayMode->blockSignals(true);
    d->btPlayMode->setMode(mode);
    d->btPlayMode->blockSignals(false);
    d->mode = mode;

    auto hintWidget = d->btPlayMode->property("HintWidget").value<Dtk::Widget::DToast *>();
    QString playmode;
    switch (mode) {
    default:
    case 0:
        playmode = Footer::tr("List Loop");
        break;
    case 1:
        playmode = Footer::tr("Single Loop");
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
    if (d->activingMeta && d->activingMeta == meta) {
        d->title->setText(meta->title);
        if (!meta->artist.isEmpty()) {
            d->artist->setText(meta->artist);
        } else {
            d->artist->setText(tr("Unknown artist"));
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
    DWidget::resizeEvent(event);

    //auto fix = progressExtentHeight();
    //auto center = rect().marginsRemoved(QMargins(0, fix, 0, 0)).center() - d->ctlWidget->rect().center();

    //d->ctlWidget->move(center);
    //d->ctlWidget->raise();
}
