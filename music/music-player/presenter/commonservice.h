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
/*
* @bref: CManagerAttributeService 用于中转单选或者多选属性展示与设置
*/
class CommonService : public QObject
{
    Q_OBJECT
public:
    enum PlayClassification {
        Music_Null,
        Music_Album,
        Music_Artist,
        Music_All_Music,
        Music_My_Collection,
        Music_Songlist,
    };
    enum PlayMode {
        playing,
        suspend,      //暂停
    };
public:
    static CommonService *getInstance();
    void setPlayStatue(PlayClassification classification, PlayMode mode);
    PlayClassification getPlayClassification() const;

    bool containsStr(QString searchText, QString text);
signals:
    void playStatueChanged(PlayClassification classification, PlayMode mode, QString songlistName);  //status
    //hashOrSearchword为hash值或者搜索关键词
    void switchToView(ListPageSwitchType switchtype, QString hashOrSearchword); //switch to playlist view,
    // 收藏音乐
    void favoriteMusic(const MediaMeta meta);
private:

private:
    CommonService();
    static CommonService *instance;
    PlayClassification m_PlayClassification;
    PlayMode m_PlayMode;
};
#endif // COMMONSERVICE_H
