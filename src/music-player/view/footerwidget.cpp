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
#include <DPushButton>
#include <DProgressBar>
#include <DFloatingWidget>
#include <DPalette>
#include <DButtonBox>

#include <metadetector.h>

#include "../musicapp.h"
#include "../core/playlistmanager.h"
#include "../core/player.h"
#include "../core/metasearchservice.h"
#include "../core/musicsettings.h"

#include "widget/filter.h"
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

DGUI_USE_NAMESPACE

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
    DButtonBoxButton  *btPlay     = nullptr;
    DButtonBoxButton  *btPrev     = nullptr;
    DButtonBoxButton  *btNext     = nullptr;
    MusicImageButton  *btFavorite = nullptr;
    MusicImageButton  *btLyric    = nullptr;
    MusicImageButton  *btPlayList = nullptr;
    ModeButton        *btPlayMode = nullptr;
    MusicImageButton  *btSound    = nullptr;
    SoundVolume       *volSlider  = nullptr;
    DButtonBox        *ctlWidget  = nullptr;
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
    auto hintWidget = new DLabel(q->parentWidget());
    hintWidget->hide();
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
        auto hintWidget = btPlayMode->property("HintWidget").value<DLabel *>();
        hintFilter->showHitsFor(btPlayMode, hintWidget);
    });

//    q->connect(progress, &Slider::valueAccpet, q, [ = ](int value) {
//        auto range = progress->maximum() - progress->minimum();
//        Q_ASSERT(range != 0);
//        Q_EMIT q->changeProgress(value, range);
//    });

    q->connect(waveform, &Waveform::valueAccpet, q, [ = ](int value) {
        auto range = waveform->maximum() - waveform->minimum();
        Q_ASSERT(range != 0);
        Q_EMIT q->changeProgress(value, range);
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

    setFixedHeight(70);
    setFocusPolicy(Qt::ClickFocus);
    setObjectName("Footer");

    setBlurRectXRadius(18);
    setBlurRectYRadius(18);
    setRadius(30);
    setMode(DBlurEffectWidget::GaussianBlur);
    setBlurEnabled(true);
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    QColor maskColor("#F7F7F7");
    maskColor.setAlphaF(0.6);
    setMaskColor(maskColor);
    setMaskAlpha(255);

    auto mainVBoxlayout = new QVBoxLayout(this);
    mainVBoxlayout->setSpacing(0);
    mainVBoxlayout->setContentsMargins(0, 0, 0, 0);

    auto hoverFilter = new HoverFilter(this);

    auto layout = new QHBoxLayout();
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(10);

    d->btCover = new MusicPixmapButton();
    d->btCover->setObjectName("FooterCoverHover");
    d->btCover->setFixedSize(50, 50);

    d->title = new Label;
    auto titleFont = d->title->font();
    titleFont.setFamily("SourceHanSansSC-Normal");
    titleFont.setPixelSize(12);
    d->title->setFont(titleFont);
    d->title->setObjectName("FooterTitle");
    d->title->setMaximumWidth(140);
    d->title->setText(tr("Unknown Title"));
//    d->title->installEventFilter(hoverFilter);

    d->artist = new Label;
    auto artistFont = d->artist->font();
    artistFont.setFamily("SourceHanSansSC-Normal");
    artistFont.setPixelSize(11);
    d->artist->setFont(titleFont);
    d->artist->setObjectName("FooterArtist");
    d->artist->setMaximumWidth(140);
    d->artist->setText(tr("Unknown artist"));

    d->btPlay = new DButtonBoxButton("");
    d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/play_normal.svg"));
    d->btPlay->setIconSize(QSize(36, 36));
    d->btPlay->setFixedSize(40, 50);

    d->btPrev = new DButtonBoxButton("");
    d->btPrev->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/last_normal.svg"));
    d->btPrev->setIconSize(QSize(36, 36));
    d->btPrev->setObjectName("FooterActionPrev");
    d->btPrev->setFixedSize(40, 50);

    d->btNext = new DButtonBoxButton("");
    d->btNext->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/next_normal.svg"));
    d->btNext->setIconSize(QSize(36, 36));
    d->btNext->setObjectName("FooterActionNext");
    d->btNext->setFixedSize(40, 50);

    d->btFavorite = new MusicImageButton(":/mpimage/light/normal/collection_normal.svg",
                                         ":/mpimage/light/hover/collection_hover.svg",
                                         ":/mpimage/light/press/collection_press.svg");
    d->btFavorite->setPropertyPic(sPropertyFavourite, QVariant(true),
                                  ":/common/image/unfav_normal.svg",
                                  ":/common/image/unfav_hover.svg",
                                  ":/common/image/unfav_press.svg");
    d->btFavorite->setObjectName("FooterActionFavorite");
    d->btFavorite->setFixedSize(50, 50);
    d->btFavorite->setTransparent(false);

    d->btLyric = new MusicImageButton(":/mpimage/light/normal/lyric_normal.svg",
                                      ":/mpimage/light/hover/lyric_hover.svg",
                                      ":/mpimage/light/press/lyric_press.svg");
    d->btLyric->setObjectName("FooterActionLyric");
    d->btLyric->setFixedSize(50, 50);
    d->btLyric->setTransparent(false);
    d->btLyric->setCheckable(true);

    QStringList modes;
    modes << ":/mpimage/light/normal/sequential_loop_normal.svg"
          << ":/mpimage/light/normal/single_tune_circulation_normal.svg"
          << ":/mpimage/light/normal/cross_cycling_normal.svg";
    d->btPlayMode = new ModeButton;
    d->btPlayMode->setObjectName("FooterActionPlayMode");
    d->btPlayMode->setFixedSize(50, 50);
    d->btPlayMode->setModeIcons(modes);
    d->btPlayMode->setTransparent(false);

    d->btSound = new MusicImageButton(":/mpimage/light/normal/volume_normal.svg",
                                      ":/mpimage/light/hover/volume_hover.svg",
                                      ":/mpimage/light/press/volume_press.svg");
    d->btSound->setPropertyPic("volume", QVariant("mid"),
                               ":/mpimage/light/normal/volume_lessen_normal.svg",
                               ":/mpimage/light/hover/volume_lessen_hover.svg",
                               ":/mpimage/light/press/volume_lessen_press.svg");
    d->btSound->setPropertyPic("volume", QVariant("low"),
                               ":/mpimage/light/normal/volume_add_normal.svg",
                               ":/mpimage/light/hover/volume_add_hover.svg",
                               ":/mpimage/light/press/volume_add_press.svg");
    d->btSound->setPropertyPic("volume", QVariant("mute"),
                               ":/mpimage/light/normal/mute_normal.svg",
                               ":/mpimage/light/normal/mute_normal.svg",
                               ":/mpimage/light/press/mute_press.svg");
    d->btSound->setObjectName("FooterActionSound");
    d->btSound->setFixedSize(50, 50);
    d->btSound->setTransparent(false);
    d->btSound->setProperty("volume", "mid");
    d->btSound->setCheckable(true);
    d->btSound->setAutoChecked(true);

    d->btPlayList = new MusicImageButton(":/mpimage/light/normal/playlist_normal.svg",
                                         ":/mpimage/light/hover/playlist_hover.svg",
                                         ":/mpimage/light/press/playlist_press.svg");
    d->btPlayList->setObjectName("FooterActionPlayList");
    d->btPlayList->setFixedSize(50, 50);
    d->btPlayList->setTransparent(false);
    d->btPlayList->setCheckable(true);
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
    musicMetaLayout->addStretch(100);
    musicMetaLayout->addWidget(d->title);
    musicMetaLayout->addWidget(d->artist);
    musicMetaLayout->addStretch(100);

    auto metaWidget = new DFrame;
//    metaWidget->setStyleSheet("border: 1px solid red;");
    auto metaLayout = new QHBoxLayout(metaWidget);
    metaLayout->setContentsMargins(0, 0, 0, 0);
    metaLayout->setSpacing(10);
    metaLayout->addWidget(d->btCover);
    metaLayout->addLayout(musicMetaLayout);

    d->ctlWidget = new DButtonBox(this);
    d->ctlWidget->setFixedSize(120, 50);
    QList<DButtonBoxButton *> allCtlButtons;
    allCtlButtons.append(d->btPrev);
    allCtlButtons.append(d->btPlay);
    allCtlButtons.append(d->btNext);
    d->ctlWidget->setButtonList(allCtlButtons, false);

    d->waveform = new Waveform(Qt::Horizontal, (QWidget *)parent, this);
    d->waveform->setMinimum(0);
    d->waveform->setMaximum(1000);
    d->waveform->setValue(0);
    d->waveform->adjustSize();

    auto actWidget = new QWidget;
    auto actLayout = new QHBoxLayout(actWidget);
    actLayout->setMargin(0);
    actLayout->setSpacing(10);
    actLayout->addWidget(d->btFavorite, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btLyric, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btPlayMode, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btSound, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btPlayList, 0, Qt::AlignRight | Qt::AlignVCenter);

//    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
//    sp.setHorizontalStretch(33);
//    metaWidget->setSizePolicy(sp);
//    actWidget->setSizePolicy(sp);

    layout->addWidget(d->ctlWidget, 0);
    layout->addWidget(metaWidget);
    layout->addWidget(d->waveform, 100);
    layout->addWidget(actWidget, 0, Qt::AlignRight | Qt::AlignVCenter);

    auto controlFrame = new DFrame;
    controlFrame->setObjectName("FooterControlFrame");
    controlFrame->setFixedHeight(60);
    controlFrame->setLayout(layout);

    //mainVBoxlayout->addStretch();
    mainVBoxlayout->addWidget(controlFrame);

    d->title->hide();
    d->artist->hide();
//    d->btPrev->hide();
//    d->btNext->hide();
    d->btFavorite->hide();
    d->btLyric->hide();

    d->btPrev->setFocusPolicy(Qt::NoFocus);
    d->btPlay->setFocusPolicy(Qt::NoFocus);
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

    d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/play_normal.svg"));
    d->btCover->setIcon(Dtk::Widget::DHiDPIHelper::loadNxPixmap(d->defaultCover));

    bool themeFlag = false;
    int themeType = MusicSettings::value("base.play.theme").toInt(&themeFlag);
    if (!themeFlag)
        themeType = 1;
    slotTheme(themeType);
}

Footer::~Footer()
{

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
    d->waveform->setEnabled(enable);

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
    d->waveform->setProperty("viewname", viewname);
}

QString Footer::defaultCover() const
{
    Q_D(const Footer);
    return d->defaultCover;
}

void Footer::setLyricButtonChecked(bool flag)
{
    Q_D(const Footer);
    if (d->btLyric->isChecked() != flag) {
        d->btLyric->blockSignals(true);
        d->btLyric->setChecked(flag);
        d->btLyric->blockSignals(false);
    }
}

void Footer::setPlaylistButtonChecked(bool flag)
{
    Q_D(const Footer);
    if (d->btPlayList->isChecked() != flag) {
        d->btPlayList->blockSignals(true);
        d->btPlayList->setChecked(flag);
        d->btPlayList->blockSignals(false);
    }
}

void Footer::mousePressEvent(QMouseEvent *event)
{
    Q_D(Footer);
    DWidget::mousePressEvent(event);
    auto subCtlPos = d->waveform->mapFromParent(event->pos());
    if (d->waveform->rect().contains(subCtlPos)
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

    //cut image
    double windowScale = (width() * 1.0) / height();
    int imageWidth = cover.height() * windowScale;
    QImage coverImage;
    if (imageWidth > cover.width()) {
        int imageheight = cover.width() / windowScale;
        coverImage = cover.copy(0, (cover.height() - imageheight) / 2, cover.width(), imageheight);
    } else {
        int imageheight = cover.height();
        coverImage = cover.copy((cover.width() - imageWidth) / 2, 0, imageWidth, imageheight);
    }

    setSourceImage(coverImage);

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
        d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/suspend_normal.svg"));
    } else {
        d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/play_normal.svg"));
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
    d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/play_normal.svg"));
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
    d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/play_normal.svg"));
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
    d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/play_normal.svg"));
}

void Footer::onMediaLibraryClean()
{
    Q_D(Footer);
//    d->btPrev->hide();
//    d->btNext->hide();
    d->btFavorite->hide();
    d->btLyric->hide();
    enableControl(false);
}

void Footer::slotTheme(int type)
{
    Q_D(Footer);
    if (type == 1) {
        QColor maskColor("#F7F7F7");
        maskColor.setAlphaF(0.6);
        setMaskColor(maskColor);
        setMaskAlpha(255);
    } else {
        QColor maskColor("#202020");
        maskColor.setAlphaF(0.5);
        setMaskColor(maskColor);
        setMaskAlpha(255);
    }
    d->waveform->setThemeType(type);
}

void Footer::onProgressChanged(qint64 value, qint64 duration)
{
    Q_D(Footer);
    d->waveform->onProgressChanged(value, duration);
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

    auto hintWidget = d->btPlayMode->property("HintWidget").value<DLabel *>();
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

    QImage cover(d->defaultCover);
    if (d->activingMeta != nullptr) {
        auto coverData = MetaSearchService::coverData(d->activingMeta);
        if (coverData.length() > 0) {
            cover = QImage::fromData(coverData);
        }
    }
    //cut image
    double windowScale = (width() * 1.0) / height();
    int imageWidth = cover.height() * windowScale;
    QImage coverImage;
    if (imageWidth > cover.width()) {
        int imageheight = cover.width() / windowScale;
        coverImage = cover.copy(0, (cover.height() - imageheight) / 2, cover.width(), imageheight);
    } else {
        int imageheight = cover.height();
        coverImage = cover.copy((cover.width() - imageWidth) / 2, 0, imageWidth, imageheight);
    }
    setSourceImage(coverImage);
}
