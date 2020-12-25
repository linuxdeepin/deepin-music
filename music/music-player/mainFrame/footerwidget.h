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

#pragma once

#include <DBlurEffectWidget>
#include <QPointer>
#include <DFloatingWidget>
#include <DPushButton>
#include <DButtonBox>
#include <DToolButton>
#include <DIconButton>
#include <DBackgroundGroup>
#include <QShortcut>

#include "player.h"
#include <searchmeta.h>
#include "metabufferdetector.h"

class QAudioBuffer;

DWIDGET_USE_NAMESPACE
class Label;
class SoundVolume;
class MusicPixmapButton;
class Waveform;
//class MetaBufferDetector;
class HintFilter;
class QTimer;
class FooterWidget : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit FooterWidget(QWidget *parent = nullptr);
    ~FooterWidget() override;

    void slotTheme(int type);
    void updateShortcut();
    // 加载波形图
    void slotLoadDetector(const QString &hash);
private:
    void initUI(QWidget *parent = nullptr);
    void setPlayProperty(Player::PlaybackStatus status);
    void installTipHint(QWidget *widget, const QString &hintstr);
    // 移动音量控件到合适的位置
    void moveVolSlider();
    // we can wait to init until frame has displayed
    void initShortcut();
signals:
    void lyricClicked();
public slots:
    // 刷新footer背景
    void slotFlushBackground();
    // 播放队列自动收起
    void slotPlayQueueAutoHidden();
    // 点击播放按钮
    void slotPlayClick(bool click);
    // 点击播放列表按钮
    void slotPlayQueueClick(bool click);
    // 点击歌词按钮
    void slotLrcClick(bool click);
    // 点击播放模式按钮
    void slotPlayModeClick(bool click);

    void slotCoverClick(bool click);
    void slotNextClick(bool click);
    void slotPreClick(bool click);
    void slotFavoriteClick(bool click);
    // 刷新收藏按钮图标
    void fluashFavoriteBtnIcon();
    // 点击音量按钮
    void slotSoundClick(bool click);
    void slotPlaybackStatusChanged(Player::PlaybackStatus statue);
    void slotMediaMetaChanged();

    void setPlayModel(Player::PlaybackMode playModel);

    // Dbug音量变化通知
    void slotDbusVolumeChanged(double volume);
    void slotFlushSoundIcon();

    //void slotDelayAutoHide();
    // 快捷键响应
    void slotShortCutTriggered();
protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
private:
    int m_slotTheme = 0;

    DBlurEffectWidget *m_forwardWidget = nullptr;

    DToolButton  *m_btPlay     = nullptr;
    DToolButton  *m_btPrev     = nullptr;
    DToolButton  *m_btNext     = nullptr;

    MusicPixmapButton  *m_btCover     = nullptr;
    Label           *m_title      = nullptr;
    Label           *m_artist     = nullptr;
    Waveform        *m_waveform   = nullptr;
    DIconButton     *m_btFavorite = nullptr;
    DIconButton     *m_btLyric    = nullptr;
    DIconButton     *m_btPlayMode = nullptr;
    DIconButton     *m_btSound    = nullptr;
    DIconButton     *m_btPlayQueue = nullptr;

    // short cut on footer
    QShortcut           *volUpShortcut          = nullptr;
    QShortcut           *volDownShortcut        = nullptr;
    QShortcut           *nextShortcut           = nullptr;
    QShortcut           *playPauseShortcut      = nullptr;
    QShortcut           *previousShortcut       = nullptr;
    QShortcut           *muteShortcut = nullptr;

    SoundVolume        *m_volSlider  = nullptr;
    DBackgroundGroup   *m_ctlWidget  = nullptr;
    MetaBufferDetector  m_metaBufferDetector;
    HintFilter          *m_hintFilter = nullptr;

    QTimer             *m_limitRepeatClick = nullptr;
};

