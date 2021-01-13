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

#include <DListView>

DWIDGET_USE_NAMESPACE

#include "musicinfoitemdelegate.h"
#include "musiclistinfomodel.h"

#include <QString>

class MusicListInfoView : public DListView
{
    Q_OBJECT
public:
    explicit MusicListInfoView(const QString &m_hash, QWidget *parent = Q_NULLPTR);
    ~MusicListInfoView() override;

    //void setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap);
//    QPixmap getSidebarPixmap() const;

    QPixmap getPlayPixmap(bool isSelect = false);
    QList<MediaMeta> getMusicListData() const;

public slots:
    void slotUpdatePlayingIcon();
signals:
    void customSort();
public:
    void setMusicListView(QMap<QString, MediaMeta> musicinfos);
//    void onMusiclistChanged(PlaylistPtr playlist, const QString name);
    void showContextMenu(const QPoint &pos);

public slots:
    // 右键菜单添加到歌单
    void slotPlayListMenuClicked(QAction *action);
    // 右键菜单播放
    void slotPlayActionClicked(bool checked = false);
    // 右键菜单暂停
    void slotPauseActionClicked(bool checked = false);
    // 右键菜单文件管理显示
    void slotFileManagementShowClicked(bool checked = false);
    // 右键菜单歌曲信息点击
    void slotMusicInfoActionClicked(bool checked = false);
    // 右键菜单从歌单删除
    void slotRemoveSongListActionClicked(bool checked = false);
    // 右键菜单从本地删除
    void slotDeleteLocalActionClicked(bool checked = false);

    void slotRemoveSingleSong(const QString &listHash, const QString &musicHash);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void startDrag(Qt::DropActions supportedActions) Q_DECL_OVERRIDE;
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    //virtual void keyboardSearch(const QString &search) Q_DECL_OVERRIDE;

private:
    void addMedia(MediaMeta meta);
    void onDoubleClicked(const QModelIndex &index);

private:
    QString m_hash;
    // 当前选中的歌曲
    MediaMeta m_currMeta;
    MusiclistInfomodel      *m_model        = nullptr;
    MusicInfoItemDelegate   *delegate     = nullptr;

    QString                  curName      = "";
    QPixmap                 playingPixmap = QPixmap(":/mpimage/light/music1.svg");
    QPixmap                 sidebarPixmap = QPixmap(":/mpimage/light/music_withe_sidebar/music1.svg");
    QIcon                   m_icon = QIcon(":/common/image/info_cover.svg");
};
