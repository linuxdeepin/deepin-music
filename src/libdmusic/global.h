// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBAL_H
#define GLOBAL_H

#include "libdmusic_global.h"

#include <QString>
#include <QUrl>
#include <QMap>
#include <QObject>

class LIBDMUSICSHARED_EXPORT DmGlobal : public QObject
{
    Q_OBJECT
    Q_ENUMS(PlaybackStatus)
    Q_ENUMS(PlayerEngineType)
    Q_ENUMS(PlaybackMode)
    Q_ENUMS(MimeType)
    Q_ENUMS(PlaylistSortType)
public:
    enum PlaybackStatus {
        Idle = 0,
        Opening,
        Buffering,
        Playing,
        Paused,
        Stopped,
        Ended,
        Error
    };

    enum PlayerEngineType {
        QtMEDIAPLAYER = 0,
        VLC
    };

    enum PlaybackMode {
        RepeatNull = -1,
        RepeatAll,
        RepeatSingle,
        Shuffle,
    };

    enum MimeType {
        MimeTypeOther = -1,
        MimeTypeLocal,
        MimeTypeCDA,
        MimeTypeNet
    };

    enum PlaylistSortType {
        SortByNull = -1,
        SortByAddTimeASC,
        SortByTitleASC,
        SortByArtistASC,
        SortByAblumASC,
        SortByCustomASC,
        SortByAddTimeDES,
        SortByTitleDES,
        SortByArtistDES,
        SortByAblumDES,
        SortByCustomDES,
        SortByAddTime, //10
        SortByTitle,
        SortByArtist,
        SortByAblum,
        SortByCustom,
    };

    explicit DmGlobal(QObject *parent = nullptr);

    // 设置应用名称
    static void setAppName(const QString &name);
    // 获取应用名称
    static QString getAppName();
    static void initPath();
    // config路径
    static void setConfigPath(const QString &path);
    static QString configPath();
    // Cache路径
    static void setCachePath(const QString &path);
    static QString cachePath();
    // Music路径
    static void setMusicPath(const QString &path);
    static QString musicPath();
    // 默认专辑名称
    static void setUnknownAlbumText(const QString &text);
    static QString unknownAlbumText();
    // 默认艺人名称
    static void setUnknownArtistText(const QString &text);
    static QString unknownArtistText();
    // 检测是否开启Wayland
    static bool checkWaylandMode();
    // 设置Wayland
    static void setWaylandMode(bool mode);
    // 是否开启Wayland
    static bool isWaylandMode();
    // 初始化播放引擎类型
    static void initPlaybackEngineType();
    // 设置播放引擎类型
    static void setPlaybackEngineType(int type);
    // 播放引擎类型1为vlc，0为QMediaPlayer
    static int playbackEngineType();
};
Q_DECLARE_METATYPE(DmGlobal::PlaybackStatus)
Q_DECLARE_METATYPE(DmGlobal::PlayerEngineType)
Q_DECLARE_METATYPE(DmGlobal::PlaybackMode)
Q_DECLARE_METATYPE(DmGlobal::MimeType)
Q_DECLARE_METATYPE(DmGlobal::PlaylistSortType)

namespace DMusic {

struct MediaMeta {
    QString hash;
    QString localPath;
    QString cuePath;
    QString title;
    QString artist;
    QString album;
    QString lyricPath;

    QByteArray originalTitle;
    QByteArray originalArtist;
    QByteArray originalAlbum;

    QString pinyinTitle;
    QString pinyinTitleShort;
    QString pinyinArtist;
    QString pinyinArtistShort;
    QString pinyinAlbum;
    QString pinyinAlbumShort;

    QString filetype;
    DmGlobal::MimeType mmType = DmGlobal::MimeTypeLocal;

    qint64  timestamp   = 0;    // addTime;
    qint64  offset      = 0;    //msec
    qint64  length      = 1;    //msec
    qint64  size        = 1;
    qint32  track       = 0;
    QString editor;
    QString composer;
    QString creator;

    QString searchID;
    QString coverUrl    = DmGlobal::cachePath() + "/images/default_cover.png";

    bool    hasimage    = false;
    bool    favourite   = false;
    bool    invalid     = false;
    bool    loadCover   = false;
    // 在列表中时，记录是否是要删除的
    bool    toDelete    = false;

    QString codec;              //save codec

    bool inMuiltSelect = false; // runtime properties
    bool dragFlag = false;
};

struct PlaylistInfo {
    QString uuid;
    QString displayName;
    QString icon;

    QStringList sortMetas;
    QStringList sortCustomMetas;

    int     sortType    = 0;
    int     orderType   = 0;

    uint    sortID;
    bool    editmode    = false;
    bool    readonly    = false;
    bool    hide        = false;
    bool    active      = false;
    bool    playStatus  = false;

    bool    saveFalg    = true;
};

struct AlbumInfo {
    QString name;
    QString pinyin;
    QString artist;
    qint64 timestamp = 0;
    QMap<QString, MediaMeta> musicinfos;
};

struct ArtistInfo {
    QString name;
    QString pinyin;
    qint64 timestamp = 0;
    QMap<QString, MediaMeta> musicinfos;
};

}

#endif // GLOBAL_H
