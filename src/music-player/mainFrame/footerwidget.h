// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPointer>
#include <QShortcut>

#include <DBlurEffectWidget>
#include <DFloatingWidget>
#include <DPushButton>
#include <DButtonBox>
#include <DToolButton>
#include <DIconButton>
#include <DBackgroundGroup>
#include <DGuiApplicationHelper>

#include "player.h"
#include "searchmeta.h"
#include "metabufferdetector.h"

class QAudioBuffer;

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class Label;
class SoundVolume;
class MusicPixmapButton;
class Waveform;
class HintFilter;
class QTimer;
class ControlIconButton;
class FooterWidget : public DFloatingWidget
{
    Q_OBJECT
public:
    // 控件边距
    static constexpr int Margin = 5;
    // 控件高度
    static constexpr int Height = 80;
    static constexpr int CompactHeight = 50;

public:
    explicit FooterWidget(QWidget *parent = nullptr);
    ~FooterWidget() override;

    void slotTheme(int type);
    void updateShortcut();
    // 加载波形图
    void slotLoadDetector(const QString &hash);
    /**
     * @brief slotSetWaveValue 设置初始进度
     * @param step 初始播放时间ms
     * @param duration 整体时长ms
     */
    void slotSetWaveValue(int step, long duration);
    // 阻止休眠
    void screenStandby(bool isStandby);
private:
    void initUI(QWidget *parent = nullptr);
    void installTipHint(QWidget *widget, const QString &hintstr);
    // 移动音量控件到合适的位置
    void moveVolSlider();
    // we can wait to init until frame has displayed
    void initShortcut();

    void setPlayProperty(Player::PlaybackStatus status);
    // 设置按钮使能状态
    void resetBtnEnable();
    QString playModeStr(int mode);

signals:
    void lyricClicked();
public slots:
    // 刷新footer背景
    void slotFlushBackground();
    // 播放队列自动收起
    void slotPlayQueueAutoHidden();
    // 歌词自动收起
    void slotLyricAutoHidden();
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
    // 删除收藏
    void slotFavoriteRemove(const QString &musicHash);
    void flushFavoriteBtnIconAdd(const QString &hash);
    // 点击音量按钮
    void slotSoundClick(bool click);
    // 点击音量按钮鼠标是否进入
    void slotSoundMouseIn(bool in);
    void slotPlaybackStatusChanged(Player::PlaybackStatus statue);
    void slotMediaMetaChanged(MediaMeta activeMeta);

    void setPlayModel(Player::PlaybackMode playModel, bool isSignal = false);

    // Dbug音量变化通知
    void slotDbusVolumeChanged(double volume);
    void slotFlushSoundIcon();

    //void slotDelayAutoHide();
    // 快捷键响应
    void slotShortCutTriggered();
    void slotCoverUpdate(const MediaMeta &meta);
#ifdef DTKWIDGET_CLASS_DSizeMode
    // 切换紧凑模式
    void slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode);
#endif
protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    int m_slotTheme = 0;

    DBlurEffectWidget  *m_forwardWidget = nullptr;

    DButtonBox         *m_ctlWidget   = nullptr;
    DButtonBoxButton   *m_btPlay      = nullptr;
    DButtonBoxButton   *m_btPrev      = nullptr;
    DButtonBoxButton   *m_btNext      = nullptr;

    MusicPixmapButton  *m_btCover     = nullptr;
    Label              *m_title       = nullptr;
    Label              *m_artist      = nullptr;
    Waveform           *m_waveform    = nullptr;
    DIconButton        *m_btFavorite  = nullptr;
    DIconButton        *m_btLyric     = nullptr;
    DIconButton        *m_btPlayMode  = nullptr;
    ControlIconButton  *m_btSound     = nullptr;
    DIconButton        *m_btPlayQueue = nullptr;

    // shortcut on footer
    QShortcut          *volUpShortcut     = nullptr;
    QShortcut          *volDownShortcut   = nullptr;
    QShortcut          *nextShortcut      = nullptr;
    QShortcut          *playPauseShortcut = nullptr;
    QShortcut          *previousShortcut  = nullptr;
    QShortcut          *muteShortcut      = nullptr;

    SoundVolume        *m_volSlider           = nullptr;
    MetaBufferDetector  m_metaBufferDetector;
    HintFilter         *m_hintFilter          = nullptr;
    quint32             m_lastCookie          = 0;

    QTimer             *m_limitRepeatClick    = nullptr;
    // 优化：添加进度条初始化状态跟踪
    bool                m_progressBarInitialized = false;
};

