// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QScopedPointer>
#include <QPainter>
#include <DFrame>
#include <DLabel>
#include <DSlider>
#include <DToolButton>

class SoundPixmapButton;

using namespace Dtk::Widget;

class SoundVolume : public QWidget
{
    Q_OBJECT
public:
    explicit SoundVolume(QWidget *parent = nullptr);
    ~SoundVolume() override;

    int volume() const;

    void setVolume(int value);
    void flushVolumeIcon();
    // 设置鼠标状态，鼠标在音量条呼出按钮上时也不消失
    void setMouseIn(bool in);
    // 启动隐藏定时器
    void startTimer(bool start);
signals:
    void delayAutoHide();
    void sigVolumeChanged();//本地音量和静音状态改变，通知footer栏改变图标
public slots:
    void delayHide();
    void setThemeType(int type);
    void slotSoundClick();
    void slotTimeOut();
protected:
    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    virtual void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    virtual void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    virtual void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private slots:
    void slotSetVolume(int volume);
private:
    // 更新控件UI
    void updateUI(int volume);
    // 初始化背景
    void initBgImage();
private:
    DLabel      *m_volPersent = nullptr;
    DSlider     *m_volSlider           = nullptr;
    DToolButton       *m_btSound         = nullptr;
    QPixmap     bgImage;

    bool        m_mouseIn     = false;
    QTimer      *m_timer = nullptr;
};






