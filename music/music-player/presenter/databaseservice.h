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
#ifndef DATABASESERVICE_H
#define DATABASESERVICE_H

#include <QObject>
#include <QMap>
#include <QSqlDatabase>


class MediaMeta;
class PlaylistMeta;
class DataBaseService : public QObject
{
    Q_OBJECT
public:
    struct PlaylistData {
        QString uuid;
        QString displayName;
        QString icon;

        QStringList              sortMetas;
        int     sortType    = 0;
        int     orderType   = 0;

        uint     sortID;
        bool    editmode    = false;
        bool    readonly    = false;
        bool    hide        = false;
        bool    active      = false;
        bool    playStatus  = false;
    };
public:
    static DataBaseService *getInstance();
    QList<PlaylistData> allPlaylistMeta();
    QList<MediaMeta> allMusicInfos();
signals:
    void musicInfoLoadFinish();
private:
    bool createConnection();
public:
    bool m_isPerformance = true;
    bool m_isSonglistPerformance = true;
    QMap<QString, MediaMeta> m_MediaMetaMap;
    bool m_couldClear = false;
private:
    DataBaseService();
    static DataBaseService *instance;

    QList<PlaylistData> m_PlaylistMeta;
    QList<MediaMeta> m_MediaMeta;
    QSqlDatabase db;
};

#endif // DATABASESERVICE_H
