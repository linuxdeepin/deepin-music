// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHBoxLayout>

#include <DWidget>
#include <DLabel>
#include <DFloatingWidget>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE

class PlayListView;
class InotifyFiles;
class QPropertyAnimation;
class PlayQueueWidget : public DFloatingWidget
{
    Q_OBJECT
public:
    // 控件边距
    static constexpr int Margin = 5;
    // 控件高度
    static constexpr int Height = 80;
    static constexpr int CompactHeight = 50;
    // 动画持续时间
    static constexpr int AnimationDelay = 400;
public:
    explicit PlayQueueWidget(QWidget *parent = Q_NULLPTR);
    ~PlayQueueWidget() override;

    void playAnimation(const QSize &size);
    void stopAnimation();

public slots:
    void slotPlayListChanged();
    void setThemeType(int type);
#ifdef DTKWIDGET_CLASS_DSizeMode
    // 紧凑模式
    void slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode);
#endif

signals:
    void signalAutoHidden();

private slots:
    void slotClearAllClicked();
    // 刷新歌曲数量
    void slotUpdateItemCount();
    // 自动收起
    void autoHidden(QWidget *old, QWidget *now);
    // 返回导入界面，收起播放队列
    void slotAllMusicCleared();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    void initAnimation();

private:
    DLabel              *m_titleLabel      = nullptr;
    DLabel              *m_infoLabel       = nullptr;
    DLabel              *m_emptyHits       = nullptr;
    DWidget             *m_actionBar       = nullptr;
    DPushButton         *m_btClearAll      = nullptr;
    PlayListView        *m_playListView    = nullptr;
    QAction             *m_customAction    = nullptr;

    QPropertyAnimation  *m_animationToUp   = nullptr;
    QPropertyAnimation  *m_animationToDown = nullptr;
    QHBoxLayout         *m_mainLayout      = nullptr;
};
