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

#include "libdmusic_global.h"

#include <QObject>
#include <QString>
#include <QVariant>
#include <QSharedPointer>
#include <QUrl>
#include <QIcon>

class QFileInfo;

enum MIMETYPE {
    MIMETYPE_OTHER = -1,
    MIMETYPE_LOCAL,
    MIMETYPE_CDA,
    MIMETYPE_NET
};

class LIBDMUSICSHARED_EXPORT MediaMeta
{
public:
    QString hash;
    QString localPath;
    QString cuePath;
    QString title;
    QString singer;
    QString album;
    QString lyricPath;

    QString pinyinTitle;
    QString pinyinTitleShort;
    QString pinyinArtist;
    QString pinyinArtistShort;
    QString pinyinAlbum;
    QString pinyinAlbumShort;

    QString filetype;

    MIMETYPE mmType = MIMETYPE_LOCAL;

    qint64  timestamp   = 0;    // addTime;
    qint64  offset      = 0;    //msec
    qint64  length      = 1;    //msec
    qint64  size        = 1;
    qint32  track       = 0;
    QString editor;
    QString composer;
    QString creator;

    QString searchID;
    QUrl    coverUrl;

    bool    hasimage = true;
    bool    favourite   = false;
    bool    invalid     = false;
    bool    loadCover   = false;
    // 在列表中时，记录是否是要删除的
    bool    toDelete    = false;
    // 平板多选时使用
    bool    beSelect    = false;

    QString codec;              //save codec


public:
    void updateSearchIndex();
    void updateCodec(const QByteArray &codec);
    void getCoverData(const QString &tmpPath);

//    static MediaMeta fromLocalFile(const QFileInfo &fileInfo);

private:
    friend bool operator == (const MediaMeta &meta1, const MediaMeta &meta2)
    {
        if (meta1.hash == meta2.hash) {
            return true;
        } else {
            return false;
        }
    }
};

struct AlbumInfo {
    QString albumName;
    QString pinyinAlbum;
    QString singer;
    QMap<QString, MediaMeta> musicinfos;
    qint64 timestamp = 0;
};

struct SingerInfo {
    QString singerName;
    QString pinyinSinger;
    qint64 timestamp = 0;
    QMap<QString, MediaMeta> musicinfos;
};

struct customInfo {
    int sortId;
    QString songPath;
};


enum SearchType {
    none = 0,
    SearchMusic,
    SearchSinger,
    SearchAlbum,
    SearchIcon
};

// 左侧按钮选择的页面
enum ListPageSwitchType {
    NullType,
    PreType,            //上一个类型
    AlbumType,          //专辑
    SingerType,         //歌手
    AllSongListType,    //所有歌曲
    FavType,            //收藏
    CdaType,            //自定义歌单cda
    CustomType,         //自定义歌单
    SearchMusicResultType,     //搜索歌曲结果
    SearchSingerResultType,    //搜索歌手结果
    SearchAlbumResultType,     //搜索专辑结果
    AlbumSubSongListType,      //专辑二级页面
    SingerSubSongListType,     //歌手二级页面
    SearchAlbumSubSongListType,//搜索专辑二级页面
    SearchSingerSubSongListType//搜索歌手二级页面
};

Q_DECLARE_METATYPE(MediaMeta)
Q_DECLARE_METATYPE(AlbumInfo)
Q_DECLARE_METATYPE(SingerInfo)
Q_DECLARE_METATYPE(ListPageSwitchType)

namespace DMusic {

QString LIBDMUSICSHARED_EXPORT filepathHash(const QString &filepath);
QString LIBDMUSICSHARED_EXPORT sizeString(qint64 sizeByte);
QString LIBDMUSICSHARED_EXPORT lengthString(qint64 length);
}


