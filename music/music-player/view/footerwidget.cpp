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
#include <DToolTip>

#include <metadetector.h>

#include "../musicapp.h"
#include "../core/playlistmanager.h"
#include "../core/player.h"
#include "../core/metasearchservice.h"
#include "../core/musicsettings.h"
#include "../core/volumemonitoring.h"
#include "../core/util/threadpool.h"

#include "widget/filter.h"
#include "widget/modebuttom.h"
#include "widget/label.h"
#include "widget/cover.h"
#include "widget/soundvolume.h"
#include "widget/musicimagebutton.h"
#include "widget/musiciconbutton.h"

#include "widget/musicpixmapbutton.h"
#include "widget/waveform.h"
#include "playlistwidget.h"
#include "widget/musicboxbutton.h"
#include "widget/tooltips.h"

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

    DBlurEffectWidget *forwardWidget = nullptr;
    Label           *title      = nullptr;
    Label           *artist     = nullptr;

    MusicPixmapButton *btCover    = nullptr;
    MusicBoxButton  *btPlay     = nullptr;
    MusicBoxButton  *btPrev     = nullptr;
    MusicBoxButton  *btNext     = nullptr;
    MusicIconButton  *btFavorite = nullptr;
    MusicIconButton  *btLyric    = nullptr;
    MusicIconButton  *btPlayList = nullptr;
    ModeButton        *btPlayMode = nullptr;
    MusicIconButton  *btSound    = nullptr;
    SoundVolume       *volSlider  = nullptr;
    DButtonBox        *ctlWidget  = nullptr;
    Waveform          *waveform   = nullptr;
    PlayListWidget    *playListWidget         = nullptr;
    bool              showPlaylistFlag        = false;

    HintFilter          *hintFilter         = nullptr;
    HoverShadowFilter   *hoverShadowFilter  = nullptr;

    PlaylistPtr     activingPlaylist        = nullptr;
    MetaPtr         activingMeta            = nullptr;

    QString         defaultCover    = ":/common/image/info_cover.svg";

    int             mode            = -1;
    bool            enableMove      = false;

    int             m_type = 1;

    bool            btPlayingStatus = false;

    VolumeMonitoring         volumeMonitoring;
    int             m_Volume = 0;
    int             m_Mute = 0;
    Footer *q_ptr;
    Q_DECLARE_PUBLIC(Footer)
};

void FooterPrivate::updateQssProperty(QWidget *w, const char *name, const QVariant &value)
{
    //Q_Q(Footer);
    w->setProperty(name, value);
    w->update();
}

void FooterPrivate::installTipHint(QWidget *w, const QString &hintstr)
{
    Q_Q(Footer);
    // TODO: parent must be mainframe
    auto hintWidget = new ToolTips("", q->parentWidget());
    hintWidget->hide();
    hintWidget->setText(hintstr);
    hintWidget->setFixedHeight(32);
//    hintWidget->setForegroundRole(DPalette::TextTitle);
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
        auto hintWidget = btPlayMode->property("HintWidget").value<QWidget *>();
        hintFilter->showHitsFor(btPlayMode, hintWidget);
    });

    q->connect(waveform, &Waveform::valueAccpet, q, [ = ](int value) {
        auto range = waveform->maximum() - waveform->minimum();
        Q_ASSERT(range != 0);
        Q_EMIT q->changeProgress(value, range);
    });

    q->connect(btPlay, &DPushButton::released, q, [ = ]() {
        q->onTogglePlayButton();
    });

    q->connect(q, &Footer::focusButton, q, [ = ]() {

        Q_EMIT btPlayList->click();
    });


    q->connect(btPrev, &DPushButton::released, q, [ = ]() {
        Q_EMIT q->prev(activingPlaylist, activingMeta);
    });
    q->connect(btNext, &DPushButton::released, q, [ = ]() {
        Q_EMIT q->next(activingPlaylist, activingMeta);
    });

    q->connect(btFavorite, &DPushButton::released, q, [ = ]() {
        if (activingMeta != nullptr)
            Q_EMIT q->toggleFavourite(activingMeta);
    });
    q->connect(title, &Label::clicked, q, [ = ](bool) {
        Q_EMIT q->locateMusic(activingPlaylist, activingMeta);
        Q_EMIT q->togglePlaylist(true);
    });
    q->connect(btLyric, &DPushButton::released, q, [ = ]() {
        Q_EMIT  q->toggleLyricView();
    });
    q->connect(btPlayList, &DPushButton::released, q, [ = ]() {
        Q_EMIT q->togglePlaylist(false);
    });
    q->connect(btSound, &DPushButton::pressed, q, [ = ]() {
        // Q_EMIT q->localToggleMute();

        auto hintWidget = btSound->property("HintWidget").value<QWidget *>();
        hintFilter->showHitsFor(btSound, hintWidget);
    });

    q->connect(volSlider, &SoundVolume::volumeMute, q, [ = ]() {
        Q_EMIT q->localToggleMute();
    });

    q->connect(volSlider, &SoundVolume::volumeChanged, q, [ = ](int vol) {
        q->onVolumeChanged(vol);
        if (m_Mute) {
            m_Mute = false;
            Q_EMIT q->toggleMute();
        }
        m_Volume = vol;
        Q_EMIT q->volumeChanged(vol);
    });

    q->connect(q, &Footer::mouseMoving, q, [ = ](Qt::MouseButton) {
        hintFilter->hideAll();
    });

    q->connect(q, &Footer::audioBufferProbed, waveform, &Waveform::onAudioBufferProbed);
    q->connect(q, &Footer::metaBuffer, waveform, &Waveform::onAudioBuffer);

    q->connect(&volumeMonitoring, &VolumeMonitoring::volumeChanged, q, [ = ](int vol) {
        q->onVolumeChanged(vol);
    });

    q->connect(&volumeMonitoring, &VolumeMonitoring::muteChanged, q, [ = ](bool mute) {
        q->onMutedChanged(mute);
    });
}

Footer::Footer(QWidget *parent) :
    DFloatingWidget(parent), d_ptr(new FooterPrivate(this))
{
    Q_D(Footer);

    //setFixedHeight(70);
    setFocusPolicy(Qt::ClickFocus);
    setObjectName("Footer");
    this->setBlurBackgroundEnabled(true);

//    this->blurBackground()->setBlurRectXRadius(18);
//    this->blurBackground()->setBlurRectYRadius(18);
    this->blurBackground()->setRadius(30);
    this->blurBackground()->setBlurEnabled(true);
    this->blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);
    QColor backMaskColor(255, 255, 255, 140);
    this->blurBackground()->setMaskColor(backMaskColor);

    d->forwardWidget = new DBlurEffectWidget(this);
//    d->forwardWidget->setBlurBackgroundEnabled(true);
    d->forwardWidget->setBlurRectXRadius(18);
    d->forwardWidget->setBlurRectYRadius(18);
    d->forwardWidget->setRadius(30);
    d->forwardWidget->setBlurEnabled(true);
    d->forwardWidget->setMode(DBlurEffectWidget::GaussianBlur);

    QColor maskColor(255, 255, 255, 76);
    d->forwardWidget->setMaskColor(maskColor);

    auto backLayout = new QVBoxLayout(this);
    backLayout->setSpacing(0);
    backLayout->setContentsMargins(0, 0, 0, 0);
    backLayout->addWidget(d->forwardWidget);

    auto mainVBoxlayout = new QVBoxLayout(d->forwardWidget);
    mainVBoxlayout->setSpacing(0);
    mainVBoxlayout->setContentsMargins(10, 0, 0, 10);

    auto downWidget = new DWidget();
    auto layout = new QHBoxLayout(downWidget);
    layout->setContentsMargins(0, 0, 10, 0);
//    layout->setSpacing(10);

    d->btCover = new MusicPixmapButton();
    d->btCover->setObjectName("FooterCoverHover");
    d->btCover->setFixedSize(50, 50);
    d->btCover->setIconSize(QSize(50, 50));


    d->title = new Label;
    auto titleFont = d->title->font();
    titleFont.setFamily("SourceHanSansSC");
    titleFont.setWeight(QFont::Normal);
    titleFont.setPixelSize(12);
    d->title->setFont(titleFont);
    d->title->setObjectName("FooterTitle");
    d->title->setMaximumWidth(140);
    d->title->setText(tr("Unknown Title"));
//    d->title->installEventFilter(hoverFilter);
//    auto titlePl = d->title->palette();
//    titlePl.setColor(DPalette::WindowText, QColor("#000000"));
//    d->title->setPalette(titlePl);

    d->title->setForegroundRole(DPalette::BrightText);

    d->artist = new Label;
    auto artistFont = d->artist->font();
    artistFont.setFamily("SourceHanSansSC");
    artistFont.setWeight(QFont::Normal);
    artistFont.setPixelSize(11);
    d->artist->setFont(titleFont);
    d->artist->setObjectName("FooterArtist");
    d->artist->setMaximumWidth(140);
    d->artist->setText(tr("Unknown artist"));
    auto artistPl = d->title->palette();
    QColor artistColor = artistPl.color(DPalette::BrightText);
    artistColor.setAlphaF(0.6);
    artistPl.setColor(DPalette::WindowText, artistColor);
    d->artist->setPalette(artistPl);
    d->artist->setForegroundRole(DPalette::WindowText);
//    d->artist->setForegroundRole(DPalette::BrightText);

    d->btPlay = new MusicBoxButton("", ":/mpimage/light/normal/play_normal.svg",
                                   ":/mpimage/light/normal/play_normal.svg",
                                   ":/mpimage/light/press/play_press.svg");
    //d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/play_normal.svg"));
    d->btPlay->setIconSize(QSize(36, 36));
    d->btPlay->setFixedSize(40, 50);

    d->btPrev = new MusicBoxButton("", ":/mpimage/light/normal/last_normal.svg",
                                   ":/mpimage/light/normal/last_normal.svg",
                                   ":/mpimage/light/press/last_press.svg");
    //d->btPrev->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/last_normal.svg"));
    d->btPrev->setIconSize(QSize(36, 36));
    d->btPrev->setObjectName("FooterActionPrev");
    d->btPrev->setFixedSize(40, 50);

    d->btNext = new MusicBoxButton("", ":/mpimage/light/normal/next_normal.svg",
                                   ":/mpimage/light/normal/next_normal.svg",
                                   ":/mpimage/light/press/next_press.svg");
    //d->btNext->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/next_normal.svg"));
    d->btNext->setIconSize(QSize(36, 36));
    d->btNext->setObjectName("FooterActionNext");
    d->btNext->setFixedSize(40, 50);

    d->btFavorite = new MusicIconButton(":/mpimage/light/normal/collection_normal.svg",
                                        ":/mpimage/light/normal/collection_normal.svg",
                                        ":/mpimage/light/press/collection_press.svg");
    d->btFavorite->setPropertyPic(sPropertyFavourite, QVariant(true),
                                  ":/mpimage/light/checked/collection1_checked.svg",
                                  ":/mpimage/light/checked/collection1_checked.svg",
                                  ":/mpimage/light/press/collection_press.svg");
    d->btFavorite->setObjectName("FooterActionFavorite");
    d->btFavorite->setShortcut(QKeySequence::fromString("."));
    d->btFavorite->setFixedSize(50, 50);
    d->btFavorite->setTransparent(false);

    d->btLyric = new MusicIconButton(":/mpimage/light/normal/lyric_normal.svg",
                                     ":/mpimage/light/hover/lyric_hover.svg",
                                     ":/mpimage/light/press/lyric_press.svg",
                                     ":/mpimage/light/checked/lyric_checked.svg");
    d->btLyric->setObjectName("FooterActionLyric");
    d->btLyric->setFixedSize(50, 50);
    d->btLyric->setTransparent(false);
    d->btLyric->setCheckable(true);

    QStringList modes;
    modes << ":/mpimage/light/normal/sequential_loop_normal.svg"
          << ":/mpimage/light/normal/single_tune_circulation_normal.svg"
          << ":/mpimage/light/normal/cross_cycling_normal.svg";
    QStringList pressModes;
    pressModes << ":/mpimage/light/press/sequential_loop_press.svg"
               << ":/mpimage/light/press/single_tune_circulation_press.svg"
               << ":/mpimage/light/press/cross_cycling_press.svg";
    d->btPlayMode = new ModeButton;
    d->btPlayMode->setObjectName("FooterActionPlayMode");
    d->btPlayMode->setFixedSize(50, 50);
    d->btPlayMode->setModeIcons(modes, pressModes);
    d->btPlayMode->setTransparent(false);

    d->btSound = new MusicIconButton(":/mpimage/light/normal/volume_normal.svg",
                                     ":/mpimage/light/hover/volume_hover.svg",
                                     ":/mpimage/light/press/volume_press.svg",
                                     ":/mpimage/light/checked/volume_checked.svg");
    d->btSound->setPropertyPic("volume", QVariant("mid"),
                               ":/mpimage/light/normal/volume_mid_normal.svg",
                               ":/mpimage/light/hover/volume_mid_hover.svg",
                               ":/mpimage/light/press/volume_mid_press.svg",
                               ":/mpimage/light/checked/volume_mid_checked.svg");
    d->btSound->setPropertyPic("volume", QVariant("low"),
                               ":/mpimage/light/normal/volume_low_normal.svg",
                               ":/mpimage/light/hover/volume_low_hover.svg",
                               ":/mpimage/light/press/volume_low_press.svg",
                               ":/mpimage/light/checked/volume_low_checked.svg");
    d->btSound->setPropertyPic("volume", QVariant("mute"),
                               ":/mpimage/light/normal/mute_normal.svg",
                               ":/mpimage/light/normal/mute_normal.svg",
                               ":/mpimage/light/press/mute_press.svg",
                               ":/mpimage/light/checked/mute_checked.svg");
    d->btSound->setObjectName("FooterActionSound");
    d->btSound->setFixedSize(50, 50);
    d->btSound->setTransparent(false);
    d->btSound->setProperty("volume", "mid");
    d->btSound->setCheckable(true);
    d->btSound->setAutoChecked(true);

    d->btPlayList = new MusicIconButton(":/mpimage/light/normal/playlist_normal.svg",
                                        ":/mpimage/light/hover/playlist_hover.svg",
                                        ":/mpimage/light/press/playlist_press.svg",
                                        ":/mpimage/checked/press/playlist_checked.svg");
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
    d->installTipHint(d->btPlayList, tr("Play Queue"));

//    d->btPrev->setToolTip(tr("Previous"));
//    d->btNext->setToolTip(tr("Next"));
//    d->btPlay->setToolTip(tr("Play/Pause"));
//    d->btFavorite->setToolTip(tr("Favorite"));
//    d->btLyric->setToolTip(tr("Lyrics"));
//    d->btPlayMode->setToolTip(tr("Play Mode"));
//    d->btPlayList->setToolTip(tr("Playlist"));

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

    auto metaLayout = new QHBoxLayout();
    metaLayout->setContentsMargins(0, 0, 0, 0);
//    metaLayout->setSpacing(10);
    metaLayout->addWidget(d->btCover);
    metaLayout->addLayout(musicMetaLayout);

    d->ctlWidget = new DButtonBox(this);
    d->ctlWidget->setFixedHeight(50);
    QList<DButtonBoxButton *> allCtlButtons;
    allCtlButtons.append(d->btPrev);
    allCtlButtons.append(d->btPlay);
    allCtlButtons.append(d->btNext);
    d->ctlWidget->setButtonList(allCtlButtons, false);

    d->waveform = new Waveform(Qt::Horizontal, static_cast<QWidget *>(parent), this);
    d->waveform->setMinimum(0);
    d->waveform->setMaximum(1000);
    d->waveform->setValue(0);
    d->waveform->adjustSize();

    auto actWidget = new QWidget;
    auto actLayout = new QHBoxLayout(actWidget);
    actLayout->setMargin(0);
//    actLayout->setSpacing(10);
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
    layout->addLayout(metaLayout);
    layout->addWidget(d->waveform, 100);
    layout->addWidget(actWidget, 0, Qt::AlignRight | Qt::AlignVCenter);

    d->playListWidget = new PlayListWidget(d->forwardWidget);
//    d->playListWidget->setContentsMargins(0, 0, 0, 0);
    d->playListWidget->hide();

//    mainVBoxlayout->addWidget(d->playListWidget);
    mainVBoxlayout->addStretch();
    mainVBoxlayout->addWidget(downWidget, 0, Qt::AlignBottom);

    d->title->hide();
    d->artist->hide();
//    d->btPrev->hide();
//    d->btNext->hide();
    d->btFavorite->hide();
    d->btLyric->hide();


    d->btFavorite->setFocusPolicy(Qt::TabFocus);
    d->btFavorite->setDefault(true);
    d->btLyric->setFocusPolicy(Qt::TabFocus);
    d->btLyric->setDefault(true);
    d->btPlayMode ->setFocusPolicy(Qt::TabFocus);
    d->btPlayMode->setDefault(true);
    d->btSound->setFocusPolicy(Qt::TabFocus);
    d->btSound->setDefault(true);
    d->btPlayList->setFocusPolicy(Qt::TabFocus);
    d->btPlayList->setDefault(true);

    d->btPrev->setDisabled(true);
    d->btNext->setDisabled(true);
    d->btPlay->setDisabled(true);

    d->initConnection();

    connect(d->btCover, &DPushButton::clicked, this, [ = ](bool) {
        Q_EMIT toggleLyricView();
        if (d->btCover->property("viewname").toString() != "lyric") {
            d->updateQssProperty(d->btCover, "viewname", "lyric");
        } else {
            d->updateQssProperty(d->btCover, "viewname", "musiclist");
        }
    });
    if (d->m_type == 1) {
        d->btPlay->setPropertyPic(":/mpimage/light/normal/play_normal.svg",
                                  ":/mpimage/light/normal/play_normal.svg",
                                  ":/mpimage/light/press/play_press.svg");
        //d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/play_press.svg"));
    } else {
        d->btPlay->setPropertyPic(":/mpimage/dark/normal/play_normal.svg",
                                  ":/mpimage/dark/normal/play_normal.svg",
                                  ":/mpimage/dark/press/play_press.svg");
        //d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/dark/normal/play_press.svg"));
    }
    d->btCover->setIcon(Dtk::Widget::DHiDPIHelper::loadNxPixmap(d->defaultCover));
    int themeType = DGuiApplicationHelper::instance()->themeType();
    slotTheme(themeType);
    d->m_Mute   = MusicSettings::value("base.play.mute").toBool();
    d->m_Volume = MusicSettings::value("base.play.volume").toInt();
    onMutedChanged(d->m_Mute);
    onVolumeChanged(d->m_Volume);

    ThreadPool::instance()->moveToNewThread(&d->volumeMonitoring);
    d->volumeMonitoring.start();
}

Footer::~Footer()
{

}

void Footer::setCurPlaylist(PlaylistPtr playlist)
{
    Q_D(Footer);
    d->activingPlaylist = playlist;
    if (d->activingPlaylist != nullptr) {
        if (d->activingPlaylist->allmusic().isEmpty()) {
            d->btPlay->setDisabled(true);
            d->btPrev->setDisabled(true);
            d->btNext->setDisabled(true);
        } else if (d->activingPlaylist->allmusic().size() == 1) {
            d->btPrev->setDisabled(true);
            d->btNext->setDisabled(true);
            d->btPlay->setDisabled(false);
        } else {
            d->btPrev->setDisabled(false);
            d->btNext->setDisabled(false);
            d->btPlay->setDisabled(false);
        }
    }
}

void Footer::enableControl(bool enable)
{
    Q_D(Footer);

    d->btCover->setEnabled(enable);
//    d->btPrev->setEnabled(enable);
//    d->btNext->setEnabled(enable);
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

//void Footer::initData(PlaylistPtr current, int mode)
//{
//    Q_D(Footer);
//    d->mode = mode;
//    d->activingPlaylist = current;
//    d->btPlayMode->setMode(mode);
//}

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

PlayListWidget *Footer::getPlayListWidget()
{
    Q_D(const Footer);
    return d->playListWidget;
}

void Footer::showPlayListWidget(int width, int height, bool changFlag)
{
    Q_D(Footer);
    if (changFlag) {
        if (d->showPlaylistFlag) {
            d->playListWidget->hide();
            setGeometry(5, height - 86, width - 10, 80);
            d->playListWidget->setGeometry(0, 0, width - 10, 0);
        } else {
            d->playListWidget->show();
            setGeometry(5, height - 429, width - 10, 423);
            d->playListWidget->setGeometry(0, 0, width - 10, 314);
        }
        d->showPlaylistFlag = (!d->showPlaylistFlag);
    } else {
        if (d->showPlaylistFlag) {
            setGeometry(5, height - 429, width - 10, 423);
            d->playListWidget->setGeometry(0, 0, width - 10, 314);
        } else {
            setGeometry(5, height - 86, width - 10, 80);
            d->playListWidget->setGeometry(0, 0, width - 10, 0);
        }
    }
}
void Footer::setSize(int width, int height, bool changFlag)
{
    Q_D(Footer);
    if (changFlag) {
        if (d->showPlaylistFlag) {
            d->playListWidget->hide();
            setFixedSize(width - 10, 80);
            move(5, height - 86);
            resize(width - 10, 80);
        } else {
            d->playListWidget->show();
            setFixedSize(width - 10, 423);
            move(5, height - 429);
            resize(width - 10, 423);
        }
        d->showPlaylistFlag = (!d->showPlaylistFlag);
    } else {
        if (d->showPlaylistFlag) {
            setFixedSize(width - 10, 423);
            move(5, height - 429);
            resize(width - 10, 423);
        } else {
            setFixedSize(width - 10, 80);
            move(5, height - 86);
            resize(width - 10, 80);
        }
    }
}

bool Footer::getShowPlayListFlag()
{
    Q_D(const Footer);
    return d->showPlaylistFlag;
}

void Footer::refreshBackground()
{
    Q_D(const Footer);
    QImage cover(d->defaultCover);
    if (d->activingMeta != nullptr) {
        auto coverData = MetaSearchService::coverData(d->activingMeta);
        if (coverData.length() > 0) {
            cover = QImage::fromData(coverData);
        }
    }
    //cut image
    double windowScale = (width() * 1.0) / height();
    int imageWidth = static_cast<int>(cover.height() * windowScale);
    QImage coverImage;
    if (d->playListWidget->isVisible()) {
        coverImage.fill(QColor(255, 255, 255));
    } else {
        if (imageWidth > cover.width()) {
            int imageheight = static_cast<int>(cover.width() / windowScale);
            coverImage = cover.copy(0, (cover.height() - imageheight) / 2, cover.width(), imageheight);
        } else {
            int imageheight = cover.height();
            coverImage = cover.copy((cover.width() - imageWidth) / 2, 0, imageWidth, imageheight);
        }
    }
    d->forwardWidget->setSourceImage(coverImage);
}

void Footer::hidewaveform()
{
    Q_D(Footer);
    d->waveform->hidewaveformScale();
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
    Q_UNUSED(metalist)
    Q_D(Footer);
    if (playlist != nullptr && playlist->id() == FavMusicListID
            && d->activingMeta != nullptr && playlist->contains(d->activingMeta))
        d->updateQssProperty(d->btFavorite, sPropertyFavourite, true);
    else {
        if (d->activingMeta != nullptr) {
            d->updateQssProperty(d->btFavorite, sPropertyFavourite, d->activingMeta->favourite);
        } else {
            d->updateQssProperty(d->btFavorite, sPropertyFavourite, false);
        }
    }

    if (d->activingPlaylist != nullptr) {
        if (d->activingPlaylist->allmusic().isEmpty()) {
            d->btPlay->setDisabled(true);
            d->btPrev->setDisabled(true);
            d->btNext->setDisabled(true);
        } else if (d->activingPlaylist->allmusic().size() == 1) {
            d->btPrev->setDisabled(true);
            d->btNext->setDisabled(true);
            d->btPlay->setDisabled(false);
        } else {
            d->btPrev->setDisabled(false);
            d->btNext->setDisabled(false);
            d->btPlay->setDisabled(false);
        }
    }
}

void Footer::onMusicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_UNUSED(metalist)
    Q_D(Footer);
    if (playlist != nullptr && playlist->id() == FavMusicListID
            && d->activingMeta != nullptr && playlist->contains(d->activingMeta))
        d->updateQssProperty(d->btFavorite, sPropertyFavourite, true);
    else {
        d->updateQssProperty(d->btFavorite, sPropertyFavourite, false);
    }

    if (d->activingPlaylist != nullptr) {
        if (d->activingPlaylist->allmusic().isEmpty()) {
            d->btPlay->setDisabled(true);
            d->btPrev->setDisabled(true);
            d->btNext->setDisabled(true);
        } else if (d->activingPlaylist->allmusic().size() == 1) {
            d->btPrev->setDisabled(true);
            d->btNext->setDisabled(true);
            d->btPlay->setDisabled(false);
        } else {
            d->btPrev->setDisabled(false);
            d->btNext->setDisabled(false);
            d->btPlay->setDisabled(false);
        }
    }
}

void Footer::onMusicPlayed(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_UNUSED(playlist)
    Q_D(Footer);

    if (!d->activingPlaylist->contains(meta))
        return;

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
    d->waveform->clearBufferAudio(meta->hash);

    //cut image
    double windowScale = (width() * 1.0) / height();
    int imageWidth = static_cast<int>(cover.height() * windowScale);
    QImage coverImage;
    if (imageWidth > cover.width()) {
        int imageheight = static_cast<int>(cover.width() / windowScale);
        coverImage = cover.copy(0, (cover.height() - imageheight) / 2, cover.width(), imageheight);
    } else {
        int imageheight = cover.height();
        coverImage = cover.copy((cover.width() - imageWidth) / 2, 0, imageWidth, imageheight);
    }

    refreshBackground();
//    d->forwardWidget->setSourceImage(coverImage);
//    blurBackground()->setSourceImage(coverImage);
    //d->waveform->onAudioBuffer(MetaDetector::getMetaData(meta->localPath));

    this->enableControl(true);
    d->title->show();
    d->artist->show();
    d->btPrev->show();
    d->btNext->show();
    d->btFavorite->show();
    d->btLyric->show();

    //d->activingPlaylist = playlist;
    d->activingMeta = meta;

    if (d->activingPlaylist != nullptr) {
        if (d->activingPlaylist->allmusic().isEmpty()) {
            d->btPlay->setDisabled(true);
            d->btPrev->setDisabled(true);
            d->btNext->setDisabled(true);
        } else if (d->activingPlaylist->allmusic().size() == 1) {
            d->btPrev->setDisabled(true);
            d->btNext->setDisabled(true);
            d->btPlay->setDisabled(false);
        } else {
            d->btPrev->setDisabled(false);
            d->btNext->setDisabled(false);
            d->btPlay->setDisabled(false);
        }
    }

    d->updateQssProperty(d->btFavorite, sPropertyFavourite, meta->favourite);

    if (!meta->invalid || true) {
        d->updateQssProperty(d->btPlay, sPropertyPlayStatus, sPlayStatusValuePlaying);
        d->updateQssProperty(this, sPropertyPlayStatus, sPlayStatusValuePlaying);
        if (d->m_type == 1) {
            d->btPlay->setPropertyPic(":/mpimage/light/normal/suspend_normal.svg",
                                      ":/mpimage/light/normal/suspend_normal.svg",
                                      ":/mpimage/light/press/suspend_press.svg");
            //d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/suspend_normal.svg"));
        } else {
            //d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/dark/normal/suspend_normal.svg"));
            d->btPlay->setPropertyPic(":/mpimage/dark/normal/suspend_normal.svg",
                                      ":/mpimage/dark/normal/suspend_normal.svg",
                                      ":/mpimage/dark/press/suspend_press.svg");
        }

        d->btPlayingStatus = true;
    } else {
        if (d->m_type == 1) {
            d->btPlay->setPropertyPic(":/mpimage/light/normal/play_normal.svg",
                                      ":/mpimage/light/normal/play_normal.svg",
                                      ":/mpimage/light/press/play_press.svg");
            //d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/play_normal.svg"));
        } else {
            d->btPlay->setPropertyPic(":/mpimage/dark/normal/play_normal.svg",
                                      ":/mpimage/dark/normal/play_normal.svg",
                                      ":/mpimage/dark/press/play_press.svg");
            //d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/dark/normal/play_normal.svg"));
        }

        d->btPlayingStatus = false;
    }
}

void Footer::onMusicError(PlaylistPtr playlist, const MetaPtr meta, int error)
{
    Q_UNUSED(playlist)
    Q_UNUSED(meta)
    Q_D(Footer);

    //d->waveform->clearBufferAudio();
//    if (d->activingMeta && d->activingPlaylist) {
//        if (meta != d->activingMeta || playlist != d->activingPlaylist) {
//            return;
//        }
//    }

    if (0 == error) {
        return;
    }

    auto status = sPlayStatusValuePause;
    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, status);
    if (d->m_type == 1) {
        d->btPlay->setPropertyPic(":/mpimage/light/normal/play_normal.svg",
                                  ":/mpimage/light/normal/play_normal.svg",
                                  ":/mpimage/light/press/play_press.svg");
        // d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/play_normal.svg"));
    } else {
        d->btPlay->setPropertyPic(":/mpimage/dark/normal/play_normal.svg",
                                  ":/mpimage/dark/normal/play_normal.svg",
                                  ":/mpimage/dark/press/play_press.svg");
        //d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/dark/normal/play_normal.svg"));
    }
    d->btPlayingStatus = false;
}

void Footer::onMusicPause(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_UNUSED(playlist)
    Q_UNUSED(meta)
    Q_D(Footer);

    auto status = sPlayStatusValuePause;
    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, status);
    if (d->m_type == 1) {
        d->btPlay->setPropertyPic(":/mpimage/light/normal/play_normal.svg",
                                  ":/mpimage/light/normal/play_normal.svg",
                                  ":/mpimage/light/press/play_press.svg");
        // d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/play_normal.svg"));
    } else {
        // d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/dark/normal/play_normal.svg"));
        d->btPlay->setPropertyPic(":/mpimage/dark/normal/play_normal.svg",
                                  ":/mpimage/dark/normal/play_normal.svg",
                                  ":/mpimage/dark/press/play_press.svg");
    }
    d->btPlayingStatus = false;

    if (d->activingPlaylist != nullptr) {
        if (d->activingPlaylist->allmusic().isEmpty()) {
            d->btPlay->setDisabled(true);
            d->btPrev->setDisabled(true);
            d->btNext->setDisabled(true);
        } else if (d->activingPlaylist->allmusic().size() == 1) {
            d->btPrev->setDisabled(true);
            d->btNext->setDisabled(true);
            d->btPlay->setDisabled(false);
        } else {
            d->btPrev->setDisabled(false);
            d->btNext->setDisabled(false);
            d->btPlay->setDisabled(false);
        }
    }
}

void Footer::onMusicStoped(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Footer);

    Q_UNUSED(playlist);
    Q_UNUSED(meta);

    onProgressChanged(0, 1, 1);
    d->title->hide();
    d->artist->hide();
    //d->btFavorite->hide();
    d->activingMeta = MetaPtr();

    d->waveform->clearBufferAudio(QString());
    d->btCover->setIcon(Dtk::Widget::DHiDPIHelper::loadNxPixmap(d->defaultCover));
    d->btCover->update();
    d->updateQssProperty(d->btPlay, sPropertyPlayStatus, sPlayStatusValueStop);
    d->updateQssProperty(this, sPropertyPlayStatus, sPlayStatusValueStop);
    if (d->m_type == 1) {
        d->btPlay->setPropertyPic(":/mpimage/light/normal/play_normal.svg",
                                  ":/mpimage/light/normal/play_normal.svg",
                                  ":/mpimage/light/press/play_press.svg");
        //d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/light/normal/play_normal.svg"));
    } else {
        //d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(":/mpimage/dark/normal/play_normal.svg"));
        d->btPlay->setPropertyPic(":/mpimage/dark/normal/play_normal.svg",
                                  ":/mpimage/dark/normal/play_normal.svg",
                                  ":/mpimage/dark/press/play_press.svg");
    }
    d->btPlayingStatus = false;

    if (d->activingPlaylist != nullptr) {
        if (d->activingPlaylist->allmusic().isEmpty()) {
            d->btPlay->setDisabled(true);
            d->btPrev->setDisabled(true);
            d->btNext->setDisabled(true);
        } else if (d->activingPlaylist->allmusic().size() == 1) {
            d->btPrev->setDisabled(true);
            d->btNext->setDisabled(true);
            d->btPlay->setDisabled(false);
        } else {
            d->btPrev->setDisabled(false);
            d->btNext->setDisabled(false);
            d->btPlay->setDisabled(false);
        }
    }
}

void Footer::onMediaLibraryClean()
{
    //Q_D(Footer);

    /*---enableControl----*/
    enableControl(false);
}

void Footer::slotTheme(int type)
{

    Q_D(Footer);
    QString rStr;
    if (type == 1) {
        QColor backMaskColor(255, 255, 255, 140);
        this->blurBackground()->setMaskColor(backMaskColor);
        QColor maskColor(255, 255, 255, 76);
        d->forwardWidget->setMaskColor(maskColor);
        rStr = "light";

        /*
        auto titlePl = d->title->palette();
        QColor titleColor("#000000");
        titlePl.setColor(DPalette::WindowText, titleColor);
        d->title->setPalette(titlePl);
        */

        auto artistPl = d->artist->palette();
        QColor artistColor = artistPl.color(DPalette::BrightText);
        artistColor.setAlphaF(0.4);
        artistPl.setColor(DPalette::WindowText, artistColor);
        d->artist->setPalette(artistPl);

        DPalette pa;
        pa = d->ctlWidget->palette();
        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        pa.setColor(DPalette::Button, QColor("#FFFFFF"));
        d->ctlWidget->setPalette(pa);

        pa = d->btFavorite->palette();
        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        d->btFavorite->setPalette(pa);

        pa = d->btLyric->palette();
        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        d->btLyric->setPalette(pa);

        pa = d->btPlayList->palette();
        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        d->btPlayList->setPalette(pa);

        pa = d->btPlayMode->palette();
        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        d->btPlayMode->setPalette(pa);

        pa = d->btSound->palette();
        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        d->btSound->setPalette(pa);

    } else {
        QColor backMaskColor(37, 37, 37, 140);
        blurBackground()->setMaskColor(backMaskColor);
        QColor maskColor(37, 37, 37, 76);
        d->forwardWidget->setMaskColor(maskColor);
        rStr = "dark";

        /*
        auto titlePl = d->title->palette();
        QColor titleColor("#FFFFFF");
        titlePl.setColor(DPalette::WindowText, titleColor);
        d->title->setPalette(titlePl);
        */

        auto artistPl = d->artist->palette();
        QColor artistColor = artistPl.color(DPalette::BrightText);
        artistColor.setAlphaF(0.6);
        artistPl.setColor(DPalette::WindowText, artistColor);
        d->artist->setPalette(artistPl);

        DPalette pa;
        pa = d->ctlWidget->palette();
        pa.setColor(DPalette::Light, QColor("#444444"));
        pa.setColor(DPalette::Dark, QColor("#444444"));
        pa.setColor(DPalette::Button, QColor("#444444"));
        d->ctlWidget->setPalette(pa);

        pa = d->btFavorite->palette();
        pa.setColor(DPalette::Light, QColor("#444444"));
        pa.setColor(DPalette::Dark, QColor("#444444"));
        d->btFavorite->setPalette(pa);

        pa = d->btLyric->palette();
        pa.setColor(DPalette::Light, QColor("#444444"));
        pa.setColor(DPalette::Dark, QColor("#444444"));
        d->btLyric->setPalette(pa);

        pa = d->btPlayList->palette();
        pa.setColor(DPalette::Light, QColor("#444444"));
        pa.setColor(DPalette::Dark, QColor("#444444"));
        d->btPlayList->setPalette(pa);

        pa = d->btPlayMode->palette();
        pa.setColor(DPalette::Light, QColor("#444444"));
        pa.setColor(DPalette::Dark, QColor("#444444"));
        d->btPlayMode->setPalette(pa);

        pa = d->btSound->palette();
        pa.setColor(DPalette::Light, QColor("#444444"));
        pa.setColor(DPalette::Dark, QColor("#444444"));
        d->btSound->setPalette(pa);

    }
    d->m_type = type;
    //d->btPlay->setIcon(DHiDPIHelper::loadNxPixmap(QString(":/mpimage/%1/normal/play_normal.svg").arg(rStr)));
    //d->btPrev->setIcon(DHiDPIHelper::loadNxPixmap(QString(":/mpimage/%1/normal/last_normal.svg").arg(rStr)));
    //d->btNext->setIcon(DHiDPIHelper::loadNxPixmap(QString(":/mpimage/%1/normal/next_normal.svg").arg(rStr)));
    if (d->btPlayingStatus) {
        d->btPlay->setPropertyPic(QString(":/mpimage/%1/normal/suspend_normal.svg").arg(rStr),
                                  QString(":/mpimage/%1/normal/suspend_normal.svg").arg(rStr),
                                  QString(":/mpimage/%1/press/suspend_press.svg").arg(rStr));
    } else {
        d->btPlay->setPropertyPic(QString(":/mpimage/%1/normal/play_normal.svg").arg(rStr),
                                  QString(":/mpimage/%1/normal/play_normal.svg").arg(rStr),
                                  QString(":/mpimage/%1/press/play_press.svg").arg(rStr));
    }
    d->btPrev->setPropertyPic(QString(":/mpimage/%1/normal/last_normal.svg").arg(rStr),
                              QString(":/mpimage/%1/normal/last_normal.svg").arg(rStr),
                              QString(":/mpimage/%1/press/last_press.svg").arg(rStr));
    d->btNext->setPropertyPic(QString(":/mpimage/%1/normal/next_normal.svg").arg(rStr),
                              QString(":/mpimage/%1/normal/next_normal.svg").arg(rStr),
                              QString(":/mpimage/%1/press/next_press.svg").arg(rStr));

    d->btFavorite->setPropertyPic(QString(":/mpimage/%1/normal/collection_normal.svg").arg(rStr),
                                  QString(":/mpimage/%1/normal/collection_normal.svg").arg(rStr),
                                  QString(":/mpimage/%1/press/collection_press.svg").arg(rStr));
    d->btFavorite->setPropertyPic(sPropertyFavourite, QVariant(true),
                                  QString(":/mpimage/%1/checked/collection1_checked.svg").arg(rStr),
                                  QString(":/mpimage/%1/checked/collection1_checked.svg").arg(rStr),
                                  QString(":/mpimage/%1/press/collection1_press.svg").arg(rStr),
                                  QString(":/mpimage/%1/checked/collection1_checked.svg").arg(rStr));

    d->btLyric->setPropertyPic(QString(":/mpimage/%1/normal/lyric_normal.svg").arg(rStr),
                               QString(":/mpimage/%1/normal/lyric_normal.svg").arg(rStr),
                               QString(":/mpimage/%1/press/lyric_press.svg").arg(rStr),
                               QString(":/mpimage/%1/checked/lyric_checked.svg").arg(rStr));
    QStringList modes;
    modes << QString(":/mpimage/%1/normal/sequential_loop_normal.svg").arg(rStr)
          << QString(":/mpimage/%1/normal/single_tune_circulation_normal.svg").arg(rStr)
          << QString(":/mpimage/%1/normal/cross_cycling_normal.svg").arg(rStr);
    QStringList pressModes;
    pressModes << QString(":/mpimage/%1/press/sequential_loop_press.svg").arg(rStr)
               << QString(":/mpimage/%1/press/single_tune_circulation_press.svg").arg(rStr)
               << QString(":/mpimage/%1/press/cross_cycling_press.svg").arg(rStr);
    d->btSound->setPropertyPic(QString(":/mpimage/%1/normal/volume_normal.svg").arg(rStr),
                               QString(":/mpimage/%1/hover/volume_hover.svg").arg(rStr),
                               QString(":/mpimage/%1/press/volume_press.svg").arg(rStr),
                               QString(":/mpimage/%1/checked/volume_checked.svg").arg(rStr));
    d->btSound->setPropertyPic("volume", QVariant("mid"),
                               QString(":/mpimage/%1/normal/volume_mid_normal.svg").arg(rStr),
                               QString(":/mpimage/%1/hover/volume_mid_hover.svg").arg(rStr),
                               QString(":/mpimage/%1/press/volume_mid_press.svg").arg(rStr),
                               QString(":/mpimage/%1/checked/volume_mid_checked.svg").arg(rStr));
    d->btSound->setPropertyPic("volume", QVariant("low"),
                               QString(":/mpimage/%1/normal/volume_low_normal.svg").arg(rStr),
                               QString(":/mpimage/%1/hover/volume_low_hover.svg").arg(rStr),
                               QString(":/mpimage/%1/press/volume_low_press.svg").arg(rStr),
                               QString(":/mpimage/%1/checked/volume_low_checked.svg").arg(rStr));
    d->btSound->setPropertyPic("volume", QVariant("mute"),
                               QString(":/mpimage/%1/normal/mute_normal.svg").arg(rStr),
                               QString(":/mpimage/%1/normal/mute_normal.svg").arg(rStr),
                               QString(":/mpimage/%1/press/mute_press.svg").arg(rStr),
                               QString(":/mpimage/%1/checked/mute_checked.svg").arg(rStr));
    d->btPlayList->setPropertyPic(QString(":/mpimage/%1/normal/playlist_normal.svg").arg(rStr),
                                  QString(":/mpimage/%1/normal/playlist_normal.svg").arg(rStr),
                                  QString(":/mpimage/%1/press/playlist_press.svg").arg(rStr),
                                  QString(":/mpimage/%1/checked/playlist_checked.svg").arg(rStr));

    d->btPlayMode->setModeIcons(modes, pressModes);
    d->waveform->setThemeType(type);
    d->volSlider->slotTheme(type);
    d->playListWidget->slotTheme(type);
}

void Footer::onTogglePlayButton()
{
    Q_D(Footer);

    auto status = d->btPlay->property(sPropertyPlayStatus).toString();
    if (d->activingMeta == nullptr) {
        if (d->activingPlaylist->isEmpty())
            return;
        d->activingMeta = d->activingPlaylist->first();
        status = "";
    }

    if (status == sPlayStatusValuePlaying) {
        Q_EMIT pause(d->activingPlaylist, d->activingMeta);
    } else  if (status == sPlayStatusValuePause) {
        Q_EMIT resume(d->activingPlaylist, d->activingMeta);
    } else {
        Q_EMIT play(d->activingPlaylist, d->activingMeta);
    }
}

void Footer::onProgressChanged(qint64 value, qint64 duration, qint64 coefficient)
{
    Q_D(Footer);

    d->waveform->onProgressChanged(value, duration, coefficient);
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
    } else {
        status = "low";
    }

    if (d->m_Mute) {
        d->updateQssProperty(d->btSound, "volume", "mute");
    } else {
        d->updateQssProperty(d->btSound, "volume", status);
    }
    d->m_Volume = volume;
    MusicSettings::setOption("base.play.volume", d->m_Volume);
    MusicSettings::setOption("base.play.mute", d->m_Mute);
    d->volSlider->onVolumeChanged(volume);
}

void Footer::onMutedChanged(bool muted)
{
    Q_D(Footer);
    if (d->volumeMonitoring.needSyncLocalFlag()) {
        d->volumeMonitoring.stop();
        d->volumeMonitoring.timeoutSlot();
        d->volumeMonitoring.start();
        return ;
    }
    d->m_Mute = muted;
    if (muted) {
        d->updateQssProperty(d->btSound, "volume", "mute");
    } else {
        QString status = "mid";
        if (d->m_Volume > 77) {
            status = "high";
        } else if (d->m_Volume > 33) {
            status = "mid";
        } else {
            status = "low";
        }

        d->updateQssProperty(d->btSound, "volume", status);
        d->volSlider->onVolumeChanged(d->m_Volume);
    }
    MusicSettings::setOption("base.play.volume", d->m_Volume);
    MusicSettings::setOption("base.play.mute", d->m_Mute);
    //sync mute to slider
    d->volSlider->syncMute(d->m_Mute);
}

void Footer::onLocalMutedChanged(int type)
{
    Q_D(Footer);
    if (type)
        d->m_Mute = !d->m_Mute;
    else
        d->m_Mute = false;

    if (d->m_Mute) {
        d->updateQssProperty(d->btSound, "volume", "mute");
    } else {
        QString status = "mid";
        if (d->m_Volume > 77) {
            status = "high";
        } else if (d->m_Volume > 33) {
            status = "mid";
        } else {
            status = "low";
        }
        d->updateQssProperty(d->btSound, "volume", status);
        d->volSlider->onVolumeChanged(d->m_Volume);
    }
    MusicSettings::setOption("base.play.volume", d->m_Volume);
    MusicSettings::setOption("base.play.mute", d->m_Mute);

    d->volumeMonitoring.syncLocalFlag();
    //emit mute state
    Q_EMIT localMuteStat(d->m_Mute);

    //sync mute to slider
    d->volSlider->syncMute(d->m_Mute);
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

    auto hintWidget = d->btPlayMode->property("HintWidget").value<QWidget *>();
    if (hintWidget != nullptr) {
        auto hintToolTips = static_cast<ToolTips *>(hintWidget);
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
        if (hintToolTips != nullptr) {
            hintToolTips->setText(playmode);
        }
    }
}

void Footer::onUpdateMetaCodec(const QString &preTitle, const QString &preArtist, const QString &preAlbum, const MetaPtr meta)
{
    Q_UNUSED(preTitle)
    Q_UNUSED(preArtist)
    Q_UNUSED(preAlbum)
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
    int imageWidth = static_cast<int>(cover.height() * windowScale);
    QImage coverImage;
    if (imageWidth > cover.width()) {
        int imageheight = static_cast<int>(cover.width() / windowScale);
        coverImage = cover.copy(0, (cover.height() - imageheight) / 2, cover.width(), imageheight);
    } else {
        int imageheight = cover.height();
        coverImage = cover.copy((cover.width() - imageWidth) / 2, 0, imageWidth, imageheight);
    }
    if (d->playListWidget->isVisible()) {
        coverImage.fill(QColor(255, 255, 255));
    }
    d->forwardWidget->setSourceImage(coverImage);
//    blurBackground()->setSourceImage(coverImage);
}
