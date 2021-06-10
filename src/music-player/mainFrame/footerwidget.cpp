/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStackedLayout>
#include <QDBusInterface>
#include <QDBusReply>
#include <QGSettings>
#include <QFrame>
#include <QShortcut>
#include <QFileInfo>
#include <QTimer>

#include <DHiDPIHelper>
#include <DPushButton>
#include <DProgressBar>
#include <DFloatingWidget>
#include <DPalette>
#include <DButtonBox>
#include <DToolTip>
#include <DBackgroundGroup>
#include <DGuiApplicationHelper>

#include <metadetector.h>
#include "../core/musicsettings.h"
#include "metabufferdetector.h"

#include "../presenter/commonservice.h"
#include "util/global.h"

#include "widget/label.h"
#include "widget/musicpixmapbutton.h"
#include "widget/waveform.h"
#include "widget/soundvolume.h"
#include "databaseservice.h"
#include "ac-desktop-define.h"
#include "tooltips.h"
#include "filter.h"
#include "mainframe.h"
#include "controliconbutton.h"

static const QString sPlayStatusValuePlaying    = "playing";
static const QString sPlayStatusValuePause      = "pause";
static const QString sPlayStatusValueStop       = "stop";
static const int VolumeStep = 10;

DGUI_USE_NAMESPACE

FooterWidget::FooterWidget(QWidget *parent) :
    DFloatingWidget(parent)
{
    setFocusPolicy(Qt::ClickFocus);
    setObjectName("FooterWidget");
    this->setBlurBackgroundEnabled(true);
    this->blurBackground()->setRadius(30);
    this->blurBackground()->setBlurEnabled(true);
    this->blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);
    QColor backMaskColor(255, 255, 255, 140);
    this->blurBackground()->setMaskColor(backMaskColor);
    initShortcut();
    initUI(parent);
    slotTheme(DGuiApplicationHelper::instance()->themeType());
    m_limitRepeatClick = new QTimer(this);
    m_limitRepeatClick->setSingleShot(true);
    m_limitRepeatClick->setInterval(200);
}

FooterWidget::~FooterWidget()
{

}

void FooterWidget::initUI(QWidget *parent)
{
    auto backLayout = new QVBoxLayout(this);
    backLayout->setSpacing(0);
    backLayout->setContentsMargins(0, 0, 0, 0);

    m_forwardWidget = new DBlurEffectWidget(this);
    m_forwardWidget->setBlurRectXRadius(18);
    m_forwardWidget->setBlurRectYRadius(18);
    m_forwardWidget->setRadius(30);
    m_forwardWidget->setBlurEnabled(true);
    m_forwardWidget->setMode(DBlurEffectWidget::GaussianBlur);
    QColor maskColor(255, 255, 255, 76);
    m_forwardWidget->setMaskColor(maskColor);
    slotFlushBackground();

//    this->layout()->addWidget(m_forwardWidget);

    auto mainHBoxlayout = new QHBoxLayout(m_forwardWidget);
    mainHBoxlayout->setSpacing(10);
    mainHBoxlayout->setContentsMargins(10, 10, 10, 10);

//    auto downWidget = new DWidget();
//    downWidget->setStyleSheet("background-color:red;");
//    auto layout = new QHBoxLayout(downWidget);
//    layout->setContentsMargins(0, 0, 10, 0);
//    mainVBoxlayout->addWidget(downWidget);

    m_btPrev = new DButtonBoxButton(QIcon::fromTheme("music_last"), "", this);
    m_btPrev->setIconSize(QSize(36, 36));
    m_btPrev->setObjectName("FooterActionPrev");
    m_btPrev->setFixedSize(40, 50);
    AC_SET_OBJECT_NAME(m_btPrev, AC_Prev);
    AC_SET_ACCESSIBLE_NAME(m_btPrev, AC_Prev);

    m_btPlay = new DButtonBoxButton(QIcon::fromTheme("music_play"), "", this);
    m_btPlay->setIconSize(QSize(36, 36));
    m_btPlay->setFixedSize(40, 50);
    AC_SET_OBJECT_NAME(m_btPlay, AC_Play);
    AC_SET_ACCESSIBLE_NAME(m_btPlay, AC_Play);

    m_btNext = new DButtonBoxButton(QIcon::fromTheme("music_next"), "", this);
    m_btNext->setIconSize(QSize(36, 36));
    m_btNext->setObjectName("FooterActionNext");
    m_btNext->setFixedSize(40, 50);
    AC_SET_OBJECT_NAME(m_btNext, AC_Next);
    AC_SET_ACCESSIBLE_NAME(m_btNext, AC_Next);

    m_ctlWidget = new DButtonBox;
    m_ctlWidget->setButtonList(QList<DButtonBoxButton *>() << m_btPrev << m_btPlay << m_btNext, false);
    mainHBoxlayout->addWidget(m_ctlWidget, 0);

    // 封面按钮
    m_btCover = new MusicPixmapButton(this);
    m_btCover->setIcon(QIcon::fromTheme("info_cover"));
    m_btCover->setObjectName("FooterCoverHover");
    m_btCover->setFixedSize(48, 48);
    m_btCover->setIconSize(QSize(48, 48));
    mainHBoxlayout->addWidget(m_btCover, 0);

    AC_SET_OBJECT_NAME(m_btCover, AC_btCover);
    AC_SET_ACCESSIBLE_NAME(m_btCover, AC_btCover);

    // 歌曲名
    m_title = new Label;
    m_title->setObjectName("FooterTitle");
    m_title->setText(tr("Unknown Title"));
    m_title->setMaximumWidth(140);
    m_title->setForegroundRole(DPalette::BrightText);
    DFontSizeManager::instance()->bind(m_title, DFontSizeManager::T8, QFont::Normal);
    // 歌唱者
    m_artist = new Label;
    m_artist->setObjectName("FooterArtist");
    m_artist->setText(tr("Unknown artist"));
    m_artist->setMaximumWidth(140);
    DFontSizeManager::instance()->bind(m_artist, DFontSizeManager::T9, QFont::Normal);

    QPalette artistPl = m_title->palette();
    QColor artistColor = artistPl.color(DPalette::BrightText);
    artistColor.setAlphaF(0.6);
    artistPl.setColor(DPalette::WindowText, artistColor);
    m_artist->setPalette(artistPl);
    m_artist->setForegroundRole(DPalette::WindowText);
    QVBoxLayout *musicMetaLayout = new QVBoxLayout;
    musicMetaLayout->setContentsMargins(0, 0, 0, 0);
    musicMetaLayout->setSpacing(0);
    musicMetaLayout->addStretch(100);
    musicMetaLayout->addWidget(m_title);
    musicMetaLayout->addWidget(m_artist);
    musicMetaLayout->addStretch(100);
    mainHBoxlayout->addLayout(musicMetaLayout);
    // 进度条
    m_waveform = new Waveform(Qt::Horizontal, static_cast<QWidget *>(parent), this);
    m_waveform->setMinimum(0);
    m_waveform->setMaximum(1000);
    m_waveform->setValue(0);
    m_waveform->adjustSize();
    mainHBoxlayout->addWidget(m_waveform, 100);

    AC_SET_OBJECT_NAME(m_waveform, AC_Waveform);
    AC_SET_ACCESSIBLE_NAME(m_waveform, AC_Waveform);

    // 收藏按钮
    m_btFavorite = new DIconButton(this);
    m_btFavorite->setObjectName("FooterActionFavorite");
    m_btFavorite->setShortcut(QKeySequence::fromString("."));
    m_btFavorite->setFixedSize(50, 50);
    m_btFavorite->setIconSize(QSize(36, 36));
    mainHBoxlayout->addWidget(m_btFavorite, 0);

    AC_SET_OBJECT_NAME(m_btFavorite, AC_Favorite);
    AC_SET_ACCESSIBLE_NAME(m_btFavorite, AC_Favorite);

    // 歌词按钮
    m_btLyric = new DIconButton(this);
    m_btLyric->setIcon(QIcon::fromTheme("lyric"));
    m_btLyric->setObjectName("FooterActionLyric");
    m_btLyric->setFixedSize(50, 50);
    m_btLyric->setIconSize(QSize(36, 36));
    m_btLyric->setCheckable(true);
    mainHBoxlayout->addWidget(m_btLyric, 0);

    AC_SET_OBJECT_NAME(m_btLyric, AC_Lyric);
    AC_SET_ACCESSIBLE_NAME(m_btLyric, AC_Lyric);

    // 播放模式
    m_btPlayMode = new DIconButton(this);
    QString playmode = "sequential_loop";
    // 根据播放管理类中模式，设置不同图标
    switch (Player::getInstance()->mode()) {
    case 0:
        playmode = "sequential_loop";
        break;
    case 1:
        playmode = "single_tune_circulation";
        break;
    case 2:
        playmode = "cross_cycling";
        break;
    default:
        break;
    }
    m_btPlayMode->setIcon(QIcon::fromTheme(playmode));
    m_btPlayMode->setObjectName("FooterActionPlayMode");
    m_btPlayMode->setFixedSize(50, 50);
    m_btPlayMode->setIconSize(QSize(36, 36));
    // 控件上记录当前播放模式
    m_btPlayMode->setProperty("playModel", QVariant(Player::getInstance()->mode()));
    mainHBoxlayout->addWidget(m_btPlayMode, 0);

    AC_SET_OBJECT_NAME(m_btPlayMode, AC_PlayMode);
    AC_SET_ACCESSIBLE_NAME(m_btPlayMode, AC_PlayMode);
    // 音量调节按钮
    m_btSound = new ControlIconButton(this);
    m_btSound->setObjectName("FooterActionSound");
    m_btSound->setFixedSize(50, 50);
    m_btSound->setCheckable(true);
    m_btSound->setIconSize(QSize(36, 36));

    mainHBoxlayout->addWidget(m_btSound, 0);

    AC_SET_OBJECT_NAME(m_btSound, AC_Sound);
    AC_SET_ACCESSIBLE_NAME(m_btSound, AC_Sound);
    // 歌曲列表按钮
    m_btPlayQueue = new DIconButton(this);
    m_btPlayQueue->setIcon(QIcon::fromTheme("playlist"));
    m_btPlayQueue->setObjectName("FooterActionPlayList");
    m_btPlayQueue->setFixedSize(50, 50);
    m_btPlayQueue->setCheckable(true);
    m_btPlayQueue->setIconSize(QSize(36, 36));
    mainHBoxlayout->addWidget(m_btPlayQueue, 0);

    AC_SET_OBJECT_NAME(m_btPlayQueue, AC_PlayQueue);
    AC_SET_ACCESSIBLE_NAME(m_btPlayQueue, AC_PlayQueue);

    // 音量控件
    m_volSlider = new SoundVolume(this->parentWidget());
    m_volSlider->hide();
    m_volSlider->setProperty("DelayHide", true);
    m_volSlider->setProperty("NoDelayShow", true);

    // 设置静音
    if (MusicSettings::value("base.play.mute").toBool()) {
        Player::getInstance()->setMuted(true);
    }

    AC_SET_OBJECT_NAME(m_volSlider, AC_VolSlider);
    AC_SET_ACCESSIBLE_NAME(m_volSlider, AC_VolSlider);

//    m_metaBufferDetector = new MetaBufferDetector(nullptr);
    connect(&m_metaBufferDetector, &MetaBufferDetector::metaBuffer, m_waveform, &Waveform::onAudioBuffer);

    // 设置提示框
    m_hintFilter =  new HintFilter(this);
    installTipHint(m_btPrev, tr("Previous"));
    installTipHint(m_btNext, tr("Next"));
    installTipHint(m_btPlay, tr("Play/Pause"));
    installTipHint(m_btFavorite, tr("Favorite"));
    installTipHint(m_btLyric, tr("Lyrics"));
    installTipHint(m_btPlayQueue, tr("Play Queue"));
    // 设置播放模式提示框
    installTipHint(m_btPlayMode, playModeStr(Player::getInstance()->mode()));

    connect(m_btPlayQueue, &DIconButton::clicked, this, &FooterWidget::slotPlayQueueClick);
    connect(m_btLyric, &DIconButton::clicked, this, &FooterWidget::slotLrcClick);
    connect(m_btPlayMode, &DIconButton::clicked, this, &FooterWidget::slotPlayModeClick);
    connect(m_btCover, &MusicPixmapButton::clicked, this, &FooterWidget::slotCoverClick);
    connect(m_btPlay, SIGNAL(clicked(bool)), this, SLOT(slotPlayClick(bool)));
    connect(m_btNext, SIGNAL(clicked(bool)), this, SLOT(slotNextClick(bool)));
    connect(m_btPrev, SIGNAL(clicked(bool)), this, SLOT(slotPreClick(bool)));
    connect(m_btSound, &DIconButton::clicked, this, &FooterWidget::slotSoundClick);
    connect(m_btSound, &ControlIconButton::mouseIn, this, &FooterWidget::slotSoundMouseIn);
    connect(m_btFavorite, &DIconButton::clicked, this, &FooterWidget::slotFavoriteClick);

    connect(Player::getInstance(), &Player::signalPlaybackStatusChanged,
            this, &FooterWidget::slotPlaybackStatusChanged);
    connect(Player::getInstance(), &Player::signalMediaMetaChanged,
            this, &FooterWidget::slotMediaMetaChanged);

    connect(CommonService::getInstance(), &CommonService::signalFluashFavoriteBtnIcon, this, &FooterWidget::fluashFavoriteBtnIcon);
    connect(CommonService::getInstance(), &CommonService::signalSetPlayModel, this, &FooterWidget::setPlayModel);
    connect(CommonService::getInstance(), &CommonService::signalPlayQueueClosed, this, &FooterWidget::slotFlushBackground);
    // dbus
    connect(Player::getInstance()->getMpris(), &MprisPlayer::volumeRequested, this, &FooterWidget::slotDbusVolumeChanged);
    connect(m_volSlider, &SoundVolume::delayAutoHide, this, [ = ]() {
        m_btSound->setChecked(false);
    });

    connect(m_volSlider, &SoundVolume::sigVolumeChanged, this, &FooterWidget::slotFlushSoundIcon);
    connect(DataBaseService::getInstance(), &DataBaseService::signalFavSongRemove, this, &FooterWidget::slotFavoriteRemove);
    connect(DataBaseService::getInstance(), &DataBaseService::signalFavSongAdd, this, &FooterWidget::flushFavoriteBtnIconAdd);

    slotFlushSoundIcon();
    resetBtnEnable();
}

void FooterWidget::installTipHint(QWidget *widget, const QString &hintstr)
{
    auto hintWidget = new ToolTips("", parentWidget()); //parentWidget()= mainframe
    hintWidget->hide();
    hintWidget->setText(hintstr);
    hintWidget->setFixedHeight(32);
    widget->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hintWidget));
    widget->installEventFilter(m_hintFilter);
}

void FooterWidget::moveVolSlider()
{
    QPoint centerPos = m_btSound->mapToGlobal(m_btSound->rect().center());
    m_volSlider->adjustSize();
    QSize sz = m_volSlider->size();
    centerPos.setX(centerPos.x()  - sz.width() / 2);
    centerPos.setY(centerPos.y() - 35 - sz.height());
    centerPos = m_volSlider->mapFromGlobal(centerPos);
    centerPos = m_volSlider->mapToParent(centerPos);
    m_volSlider->move(centerPos);
}

void FooterWidget::initShortcut()
{
    playPauseShortcut = new QShortcut(this);
    playPauseShortcut->setKey(QKeySequence(MusicSettings::value("shortcuts.all.play_pause").toString()));
    playPauseShortcut->setAutoRepeat(false);

    volUpShortcut = new QShortcut(this);
    volUpShortcut->setKey(QKeySequence(MusicSettings::value("shortcuts.all.volume_up").toString()));

    volDownShortcut = new QShortcut(this);
    volDownShortcut->setKey(QKeySequence(MusicSettings::value("shortcuts.all.volume_down").toString()));

    nextShortcut = new QShortcut(this);
    nextShortcut->setKey(QKeySequence(MusicSettings::value("shortcuts.all.next").toString()));
    nextShortcut->setAutoRepeat(false);

    previousShortcut = new QShortcut(this);
    previousShortcut->setKey(QKeySequence(MusicSettings::value("shortcuts.all.previous").toString()));
    previousShortcut->setAutoRepeat(false);

    muteShortcut = new QShortcut(this);
    muteShortcut->setKey(QKeySequence(QLatin1String("M")));
    //connect(muteShortcut, &QShortcut::activated, presenter, &Presenter::onLocalToggleMute);

    connect(playPauseShortcut, &QShortcut::activated, this, &FooterWidget::slotShortCutTriggered);
    connect(volUpShortcut, &QShortcut::activated, this, &FooterWidget::slotShortCutTriggered);
    connect(volDownShortcut, &QShortcut::activated, this, &FooterWidget::slotShortCutTriggered);
    connect(nextShortcut, &QShortcut::activated, this, &FooterWidget::slotShortCutTriggered);
    connect(previousShortcut, &QShortcut::activated, this, &FooterWidget::slotShortCutTriggered);
    connect(muteShortcut, &QShortcut::activated, this, &FooterWidget::slotShortCutTriggered);
}

void FooterWidget::updateShortcut()
{
    //it will be invoked when settings closed
    auto play_pauseStr = MusicSettings::value("shortcuts.all.play_pause").toString();
    if (play_pauseStr.isEmpty())
        playPauseShortcut->setEnabled(false);
    else {
        playPauseShortcut->setEnabled(true);
        playPauseShortcut->setKey(QKeySequence(play_pauseStr));
    }
    auto volume_upStr = MusicSettings::value("shortcuts.all.volume_up").toString();
    if (volume_upStr.isEmpty())
        volUpShortcut->setEnabled(false);
    else {
        volUpShortcut->setEnabled(true);
        volUpShortcut->setKey(QKeySequence(volume_upStr));
    }
    auto volume_downStr = MusicSettings::value("shortcuts.all.volume_down").toString();
    if (volume_downStr.isEmpty())
        volDownShortcut->setEnabled(false);
    else {
        volDownShortcut->setEnabled(true);
        volDownShortcut->setKey(QKeySequence(volume_downStr));
    }
    auto nextStr = MusicSettings::value("shortcuts.all.next").toString();
    if (nextStr.isEmpty())
        nextShortcut->setEnabled(false);
    else {
        nextShortcut->setEnabled(true);
        nextShortcut->setKey(QKeySequence(nextStr));
    }
    auto previousStr = MusicSettings::value("shortcuts.all.previous").toString();
    if (previousStr.isEmpty())
        previousShortcut->setEnabled(false);
    else {
        previousShortcut->setEnabled(true);
        previousShortcut->setKey(QKeySequence(previousStr));
    }
}
//设置播放按钮播放图标
void FooterWidget::setPlayProperty(Player::PlaybackStatus status)
{
    if (status != Player::PlaybackStatus::Playing) {
        m_btPlay->setIcon(QIcon::fromTheme("music_play"));
    } else {
        m_btPlay->setIcon(QIcon::fromTheme("suspend"));
    }
}

void FooterWidget::resetBtnEnable()
{
    // 需求变动，只针对收藏按钮做处理
    if (Player::getInstance()->getActiveMeta().hash.isEmpty()) {
        m_btFavorite->setIcon(QIcon::fromTheme("dcc_collectiondis"));
        m_btFavorite->setEnabled(false);
    } else {
        m_btFavorite->setEnabled(true);
        if (DataBaseService::getInstance()->favoriteExist(Player::getInstance()->getActiveMeta())) {
            m_btFavorite->setIcon(QIcon::fromTheme("collection1_press"));
        } else {
            m_btFavorite->setIcon(QIcon::fromTheme("dcc_collection"));
        }
    }
}
// 根据播放模式，获取中文tip
QString FooterWidget::playModeStr(int mode)
{
    QString playmode;
    switch (mode) {
    case 0:
        playmode = tr("List Loop");
        break;
    case 1:
        playmode = tr("Single Loop");
        break;
    case 2:
        playmode = tr("Shuffle");
        break;
    }
    return playmode;
}

void FooterWidget::slotPlayQueueAutoHidden()
{
    // 播放队列自动收起时更新播放队列按钮
    m_btPlayQueue->setChecked(false);
}

void FooterWidget::slotLyricAutoHidden()
{
    // 歌词控件自动收起时更新歌词按钮
    m_btLyric->setChecked(false);
}

void FooterWidget::slotPlayClick(bool click)
{
    Q_UNUSED(click)
    //限制用户反复点击
    if (!m_limitRepeatClick->isActive()) {
        m_limitRepeatClick->start(200);
    } else
        return;

    if (Player::getInstance()->status() == Player::PlaybackStatus::Playing) {
        Player::getInstance()->pause();
    } else if (Player::getInstance()->status() == Player::PlaybackStatus::Paused) {
        Player::getInstance()->resume();
    } else if (Player::getInstance()->status() == Player::PlaybackStatus::Stopped) {
        Player::getInstance()->forcePlayMeta();
    }
}

void FooterWidget::slotLrcClick(bool click)
{
    Q_UNUSED(click)
    emit lyricClicked();
}

void FooterWidget::slotPlayModeClick(bool click)
{
    Q_UNUSED(click)
#ifdef TABLET_PC
    qDebug() << __FUNCTION__ << "DGuiApplicationHelper::isTabletEnvironment() = " << DGuiApplicationHelper::isTabletEnvironment();
#endif
    int playModel = m_btPlayMode->property("playModel").toInt();
    if (++playModel == 3)
        playModel = 0;

    setPlayModel(static_cast<Player::PlaybackMode>(playModel));
    //更换提示框
    auto hintWidget = m_btPlayMode->property("HintWidget").value<QWidget *>();
    m_hintFilter->showHitsFor(m_btPlayMode, hintWidget);

    if (hintWidget != nullptr) {
        auto hintToolTips = static_cast<ToolTips *>(hintWidget);
        if (hintToolTips != nullptr) {
            hintToolTips->setText(playModeStr(playModel));
        }
    }
}

void FooterWidget::slotCoverClick(bool click)
{
    Q_UNUSED(click)
    m_btLyric->setChecked(!m_btLyric->isChecked());
    emit lyricClicked();
}

void FooterWidget::slotNextClick(bool click)
{
    Q_UNUSED(click)
    Player::getInstance()->playNextMeta(false);
}

void FooterWidget::slotPreClick(bool click)
{
    Q_UNUSED(click)
    Player::getInstance()->playPreMeta();
}

void FooterWidget::slotFavoriteClick(bool click)
{
    Q_UNUSED(click)
    bool isFavorite = DataBaseService::getInstance()->favoriteMusic(Player::getInstance()->getActiveMeta());
    fluashFavoriteBtnIcon();

    if (isFavorite)
        emit CommonService::getInstance()->signalShowPopupMessage(
            DataBaseService::getInstance()->getPlaylistNameByUUID("fav"), 1, 1);
}

void FooterWidget::fluashFavoriteBtnIcon()
{
    if (CommonService::getInstance()->getListPageSwitchType() == ListPageSwitchType::FavType)
        emit CommonService::getInstance()->signalSwitchToView(FavType, "fav");

    if (DataBaseService::getInstance()->favoriteExist(Player::getInstance()->getActiveMeta())) {
        m_btFavorite->setIcon(QIcon::fromTheme("collection1_press"));
    } else {
        m_btFavorite->setIcon(QIcon::fromTheme("dcc_collection"));
    }
}

void FooterWidget::slotFavoriteRemove(const QString &musicHash)
{
    if (musicHash == Player::getInstance()->getActiveMeta().hash) {
        m_btFavorite->setIcon(QIcon::fromTheme("dcc_collection"));
    }
}

void FooterWidget::flushFavoriteBtnIconAdd(const QString &hash)
{
    if (CommonService::getInstance()->getListPageSwitchType() == ListPageSwitchType::FavType)
        emit CommonService::getInstance()->signalSwitchToView(FavType, "fav");

    if (hash == Player::getInstance()->getActiveMeta().hash) {
        m_btFavorite->setIcon(QIcon::fromTheme("collection1_press"));
    } else {
        m_btFavorite->setIcon(QIcon::fromTheme("dcc_collection"));
    }
}

void FooterWidget::slotSoundClick(bool click)
{
    Q_UNUSED(click)
    if (m_volSlider->isVisible()) {
        m_volSlider->hide();
    } else {
        moveVolSlider();
        m_volSlider->show();
        m_volSlider->raise();
    }
}

void FooterWidget::slotSoundMouseIn(bool in)
{
    m_volSlider->setMouseIn(in);
    // 鼠标进入则关闭定时器
    m_volSlider->startTimer(!in);
}

void FooterWidget::slotPlaybackStatusChanged(Player::PlaybackStatus status)
{
    setPlayProperty(status);
    if (status == Player::PlaybackStatus::Stopped) {
        Player::getInstance()->getMpris()->setPlaybackStatus(Mpris::Stopped);
    } else {
        Player::getInstance()->getMpris()->setPlaybackStatus(status == Player::PlaybackStatus::Playing
                                                             ? Mpris::Playing : Mpris::Paused);
    }
}

void FooterWidget::slotMediaMetaChanged(MediaMeta activeMeta)
{
    Q_UNUSED(activeMeta)
    MediaMeta meta = Player::getInstance()->getActiveMeta();
    resetBtnEnable();
    //替换封面按钮与背景图片
    QString imagesDirPath = Global::cacheDir() + "/images/" + meta.hash + ".jpg";
    QFileInfo file(imagesDirPath);
    QIcon icon;
    if (file.exists()) {
        icon = QIcon(imagesDirPath);
        m_btCover->setIcon(icon);
    } else {
        m_btCover->setIcon(QIcon::fromTheme("info_cover"));
    }
    // 歌曲切换，进度条设置到初始位置
    m_waveform->setValue(0);
    slotFlushBackground();
    QFontMetrics fm(m_title->font());
    QString titleText = fm.elidedText(meta.title, Qt::ElideMiddle, m_title->maximumWidth());
    m_title->setText(titleText.isEmpty() ? tr("Unknown Title") : titleText);
    QFontMetrics singerFm(m_artist->font());
    QString singerText = singerFm.elidedText(meta.singer, Qt::ElideMiddle, m_artist->maximumWidth());
    m_artist->setText(singerText.isEmpty() ? tr("Unknown artist") : singerText);
    m_metaBufferDetector.onClearBufferDetector();
    if (!meta.hash.isEmpty()) { //避免解析无效的歌曲的波浪条数据
        m_metaBufferDetector.onBufferDetector(meta.localPath, meta.hash);
    }

    if (meta.mmType == MIMETYPE_CDA) {
        m_btFavorite->setIcon(QIcon::fromTheme("dcc_collectiondis"));
        m_btFavorite->setEnabled(false);
    } else {
        // 根据当前歌曲判断收藏是否可用
        if (Player::getInstance()->getActiveMeta().hash.isEmpty()) {
            m_btFavorite->setIcon(QIcon::fromTheme("dcc_collectiondis"));
            m_btFavorite->setEnabled(false);
        } else {
            m_btFavorite->setEnabled(true);
            if (DataBaseService::getInstance()->favoriteExist(Player::getInstance()->getActiveMeta())) {
                m_btFavorite->setIcon(QIcon::fromTheme("collection1_press"));
            } else {
                m_btFavorite->setIcon(QIcon::fromTheme("dcc_collection"));
            }
        }
    }
}

void FooterWidget::setPlayModel(Player::PlaybackMode playModel)
{
    switch (playModel) {
    case 0:
        m_btPlayMode->setIcon(QIcon::fromTheme("sequential_loop"));
        break;
    case 1:
        m_btPlayMode->setIcon(QIcon::fromTheme("single_tune_circulation"));
        break;
    case 2:
        m_btPlayMode->setIcon(QIcon::fromTheme("cross_cycling"));
        break;
    default:
        break;
    }

    m_btPlayMode->setProperty("playModel", QVariant(playModel));
    Player::getInstance()->setMode(static_cast<Player::PlaybackMode>(playModel));
}

// Dbus
void FooterWidget::slotDbusVolumeChanged(double volume)
{
    //get dbus volume
    int curVolume = int(volume * 100);
    m_volSlider->setVolume(curVolume);
}

void FooterWidget::slotPlayQueueClick(bool click)
{
    Q_UNUSED(click)

    if (!this->isHidden()) {
        m_forwardWidget->setSourceImage(QImage());
    }

    MainFrame *mainFree = static_cast<MainFrame *>(parent());
    if (mainFree) {
        mainFree->playQueueAnimation();
    }
}

void FooterWidget::slotFlushSoundIcon()
{
    m_volSlider->update();
    int volume = Player::getInstance()->getVolume();

    if (Player::getInstance()->getMuted() || volume == 0) {
        m_btSound->setIcon(QIcon::fromTheme("mute"));
    } else {
        if (volume > 77) {
            m_btSound->setIcon(QIcon::fromTheme("volume"));
        } else if (volume > 33) {
            m_btSound->setIcon(QIcon::fromTheme("volume_mid"));
        } else {
            m_btSound->setIcon(QIcon::fromTheme("volume_low"));
        }
    }

    m_btSound->update();
}

//void FooterWidget::slotDelayAutoHide()
//{
//    m_btSound->setChecked(false);
//}

void FooterWidget::slotShortCutTriggered()
{
    QShortcut *objCut = dynamic_cast<QShortcut *>(sender()) ;
    Q_ASSERT(objCut);

    if (objCut == volUpShortcut) {
        int volup = Player::getInstance()->getVolume() + VolumeStep;
        if (volup > 100)
            volup = 100;

        m_volSlider->setVolume(volup);
    }

    if (objCut == volDownShortcut) {
        int voldown = Player::getInstance()->getVolume() - VolumeStep;
        if (voldown < 0)
            voldown = 0;

        m_volSlider->setVolume(voldown);
    }

    if (objCut == nextShortcut) {
        Player::getInstance()->playNextMeta(false);
    }

    if (objCut == playPauseShortcut) { //pause
        slotPlayClick(true);
    }

    if (objCut == previousShortcut) {
        Player::getInstance()->playPreMeta();
    }

    if (objCut == muteShortcut) {
        bool mute = Player::getInstance()->getMuted();
        Player::getInstance()->setMuted(!mute);
        m_volSlider->flushVolumeIcon();
    }
}

void FooterWidget::slotLoadDetector(const QString &hash)
{
    //查找hash对应路径
    MediaMeta mt = DataBaseService::getInstance()->getMusicInfoByHash(hash);
    if (mt.localPath.isEmpty())
        return;
    m_metaBufferDetector.onBufferDetector(mt.localPath, mt.hash);
    qDebug() << __FUNCTION__ << mt.localPath << mt.hash;
}

void FooterWidget::slotSetWaveValue(int step, long duration)
{
    m_waveform->onProgressChanged(step, duration, 1); //1:偏移率
}

void FooterWidget::resizeEvent(QResizeEvent *event)
{
    if (m_forwardWidget) {
        m_forwardWidget->setGeometry(6, height() - 75, width() - 12, 70);
    }

    moveVolSlider();

    DWidget::resizeEvent(event);
}

void FooterWidget::slotFlushBackground()
{
    QImage cover = QImage(":/icons/deepin/builtin/actions/info_cover_142px.svg");
    QString imagesDirPath = Global::cacheDir() + "/images/" + Player::getInstance()->getActiveMeta().hash + ".jpg";
    QFileInfo file(imagesDirPath);
    if (file.exists()) {
        cover = QImage(Global::cacheDir() + "/images/" + Player::getInstance()->getActiveMeta().hash + ".jpg");
    }

    double windowScale = (width() * 1.0) / height();
    int imageWidth = static_cast<int>(cover.height() * windowScale);
    QImage coverImage;

    if (m_btPlayQueue && m_btPlayQueue->isChecked()) {
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

    m_forwardWidget->setSourceImage(coverImage);
    m_forwardWidget->update();
}

void FooterWidget::slotTheme(int type)
{
    m_slotTheme = type;

    // 组合按钮无边框
    QColor framecolor("#FFFFFF");
    framecolor.setAlphaF(0.00);
    QString rStr;
    if (type == 1) {
        QColor maskColor(247, 247, 247);
        maskColor.setAlphaF(0.60);
        m_forwardWidget->setMaskColor(maskColor);
        rStr = "light";

        auto artistPl = m_artist->palette();
        QColor artistColor = artistPl.color(DPalette::BrightText);
        artistColor.setAlphaF(0.4);
        artistPl.setColor(DPalette::WindowText, artistColor);
        m_artist->setPalette(artistPl);

        DPalette pa;
        pa = m_btFavorite->palette();
        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        pa.setColor(DPalette::ButtonText, QColor(Qt::black));
        // 单个按钮边框
        QColor btnframecolor("#000000");
        btnframecolor.setAlphaF(0.00);
        pa.setColor(DPalette::FrameBorder, btnframecolor);
        // 取消阴影
        pa.setColor(DPalette::Shadow, btnframecolor);
        DApplicationHelper::instance()->setPalette(m_btFavorite, pa);
        DApplicationHelper::instance()->setPalette(m_btLyric, pa);
        DApplicationHelper::instance()->setPalette(m_btPlayMode, pa);
        DApplicationHelper::instance()->setPalette(m_btSound, pa);
        DApplicationHelper::instance()->setPalette(m_btPlayQueue, pa);

        DPalette pl = m_ctlWidget ->palette();
        pl.setColor(DPalette::Button, QColor("#FFFFFF"));
        pl.setColor(DPalette::ButtonText, QColor(Qt::black));
        pl.setColor(DPalette::FrameBorder, framecolor);
        pl.setColor(DPalette::Shadow, framecolor);
        DApplicationHelper::instance()->setPalette(m_ctlWidget, pl);
    } else {
        QColor maskColor(32, 32, 32);
        maskColor.setAlphaF(0.80);
        m_forwardWidget->setMaskColor(maskColor);
        rStr = "dark";

        auto artistPl = m_artist->palette();
        QColor artistColor = artistPl.color(DPalette::BrightText);
        artistColor.setAlphaF(0.6);
        artistPl.setColor(DPalette::WindowText, artistColor);
        m_artist->setPalette(artistPl);

        DPalette pa;
        pa = m_btFavorite->palette();
        QColor btnMaskColor("#000000");
        btnMaskColor.setAlphaF(0.30);
        pa.setColor(DPalette::Light, btnMaskColor);
        pa.setColor(DPalette::Dark, btnMaskColor);
        pa.setColor(DPalette::ButtonText, QColor("#c5cfe0"));
        pa.setColor(DPalette::FrameBorder, framecolor);
        // 取消阴影
        pa.setColor(DPalette::Shadow, framecolor);
        DApplicationHelper::instance()->setPalette(m_btFavorite, pa);
        DApplicationHelper::instance()->setPalette(m_btLyric, pa);
        DApplicationHelper::instance()->setPalette(m_btPlayMode, pa);
        DApplicationHelper::instance()->setPalette(m_btSound, pa);
        DApplicationHelper::instance()->setPalette(m_btPlayQueue, pa);

        DPalette pl = m_ctlWidget ->palette();
        QColor btnColor("#000000");
        btnColor.setAlphaF(0.60);
        pl.setColor(DPalette::Button, btnColor);
        pl.setColor(DPalette::ButtonText, QColor("#c5cfe0"));
        pl.setColor(DPalette::FrameBorder, framecolor);
        pl.setColor(DPalette::Shadow, framecolor);
        DApplicationHelper::instance()->setPalette(m_ctlWidget, pl);
    }

    m_waveform->setThemeType(type);
}
