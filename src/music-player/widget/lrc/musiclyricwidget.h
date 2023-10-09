// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHBoxLayout>
#include <QImage>

#include <DWidget>
#include <DLabel>
#include <DBlurEffectWidget>

#include "searchmeta.h"
#include "player.h"

DWIDGET_USE_NAMESPACE

class Cover;
class LyricLabel;
class MusicImageButton;
class MusicLyricWidget : public DWidget
{
    Q_OBJECT

public:
    explicit MusicLyricWidget(QWidget *parent = Q_NULLPTR);
    ~MusicLyricWidget() override;

    void updateUI();
    // 控件显示动画
    void showAnimation();
    // 控件关闭动画
    void closeAnimation();

signals:
    // 歌词自动收起
    void signalAutoHidden();
    void toggleLyricView();
    void requestContextSearch(const QString &context);

public slots:
    void onProgressChanged(qint64 value, qint64 length);
    void onMusicPlayed(MediaMeta meta);
    void onCoverChanged(MediaMeta meta);
    void setThemeType(int type);

protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QHBoxLayout *m_leftLayout;
    Cover               *m_cover              = nullptr;
    LyricLabel          *m_lyricview            = nullptr;
    DLabel              *m_nolyric              = nullptr;
    bool                 m_serachflag = false;
    DBlurEffectWidget   *m_backgroundW = nullptr;
};
