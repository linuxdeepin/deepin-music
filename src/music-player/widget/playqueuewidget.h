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

#pragma once

#include <DWidget>
#include <DLabel>
#include <DFloatingWidget>

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
    DLabel              *m_titleLabel     = nullptr;
    DLabel              *m_infoLabel      = nullptr;
    DLabel              *m_emptyHits      = nullptr;
    DWidget             *m_actionBar      = nullptr;
    DPushButton         *m_btClearAll     = nullptr;
    PlayListView        *m_playListView   = nullptr;
    QAction             *m_customAction   = nullptr;

    QPropertyAnimation  *m_animationToUp = nullptr;
    QPropertyAnimation  *m_animationToDown = nullptr;
};
