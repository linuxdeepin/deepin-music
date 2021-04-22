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
    enum TabletSelectMode {
        SingleSelect = 0,
        MultSelect
    };
public:
    // 是否平板环境判断
    bool isTabletEnvironment();
    // 设置是否平板环境，单元测试覆盖平板代码时使用
    void setIsTabletEnvironment(bool isTablet);

    void setListPageSwitchType(ListPageSwitchType lpst);
    ListPageSwitchType getListPageSwitchType() const;

    bool containsStr(QString searchText, QString text);
    // 设置选择模式
    void setSelectModel(TabletSelectMode model);
    TabletSelectMode getSelectModel()const {return m_select;}
    // 横竖屏
    void setIsHScreen(bool state);
    bool isHScreen();

    void setCurrentWidgetPosY(int posY);
    int  getCurrentWidgetPosY();
signals:
    void loadData();
    // hashOrSearchword为hash值或者搜索关键词
    void signalSwitchToView(ListPageSwitchType switchtype, QString hashOrSearchword, QMap<QString, MediaMeta> musicinfos = QMap<QString, MediaMeta>()); //switch to playlist view,
    // 跳转到当前播放的位置,参数为需要跳转的歌单hash
    void sigScrollToCurrentPosition(QString songlistHash);
    // 刷新收藏按钮图标
    void signalFluashFavoriteBtnIcon();
    // 添加新的歌曲清单
    void signalAddNewSongList();
    // 添加&删除CD歌曲清单
    void signalCdaSongListChanged(int stat);
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
    // 通知页面刷新编码
    void signalUpdateCodec(const MediaMeta &meta);
    // cda导入完成消息
    void signalCdaImportFinished();
    // 选择模式
    void signalSelectMode(TabletSelectMode model);
    // 全选
    void signalSelectAll();
    // 横竖屏切换 true:横 false:竖
    void signalHScreen(bool stat);
    // 滚动
    void signalScroll(int height);
private:
    explicit CommonService();
    friend class DMusic::DSingleton<CommonService>;
private:
    ListPageSwitchType listPageSwitchType = ListPageSwitchType::AllSongListType;
    TabletSelectMode m_select = SingleSelect;
    bool m_isHScreen = false;
    int  m_currentWidgetPosY = 0;
    bool m_isTabletEnvironment = false;
};
#endif // COMMONSERVICE_H
