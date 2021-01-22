/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author: Zou Ya<zouya@uniontech.com>
*
* Maintainer: Zou Ya <zouya@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef COMMONSERVICE_H
#define COMMONSERVICE_H

#include <QObject>
#include "mediameta.h"
#include "player.h"
#include "util/singleton.h"

/**
* @bref: CManagerAttributeService 用于中转单选或者多选属性展示与设置
*/
class CommonService : public QObject, public DMusic::DSingleton<CommonService>
{
    Q_OBJECT
public:
    void setListPageSwitchType(ListPageSwitchType lpst);
    ListPageSwitchType getListPageSwitchType() const;

    bool containsStr(QString searchText, QString text);
signals:
    // hashOrSearchword为hash值或者搜索关键词
    void signalSwitchToView(ListPageSwitchType switchtype, QString hashOrSearchword); //switch to playlist view,
    // 跳转到当前播放的位置,参数为需要跳转的歌单hash
    void sigScrollToCurrentPosition(QString songlistHash);
    // 刷新收藏按钮图标
    void signalFluashFavoriteBtnIcon();
    // 添加新的歌曲清单
    void signalAddNewSongList();
    // 右键菜单播放所有音乐
    void signalPlayAllMusic();
    // 设置播放模式
    void signalSetPlayModel(Player::PlaybackMode playModel);
    // 弹窗消息
    void signalShowPopupMessage(const QString &songListName, int allCount, int successCount);
    // 播放队列关闭动画播放完毕，刷新背景
    void signalPlayQueueClosed();
    // 切换歌单时，清空搜索栏
    void signalClearEdit();
    // 双击专辑或者演唱者，显示二级页面
    void signalShowSubSonglist(const QMap<QString, MediaMeta> &musicinfos, ListPageSwitchType listPageType);
    // 隐藏二级页面
    void signalHideSubSonglist();
private:
    explicit CommonService();
    friend class DMusic::DSingleton<CommonService>;
private:
    ListPageSwitchType listPageSwitchType = ListPageSwitchType::AllSongListType;
};
#endif // COMMONSERVICE_H
