/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>
#include <QSharedPointer>
#include <QSettings>

#include "../model/musiclistmodel.h"

class Playlist : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
public:
    explicit Playlist(const MusicListInfo &musiclistinfo, QObject *parent = 0);

    enum PlayMode {
        Order  = 0,
        RepeatList,
        Repeat,
        Shufflt
    };
    Q_ENUM(PlayMode)

public:
    //! public interface
    QString id();
    QString displayName();
    QString icon();
    bool readonly();
    bool editmode();
    int length();

    const MusicInfo first();
    const MusicInfo prev(const MusicInfo &info);
    const MusicInfo next(const MusicInfo &info);

    bool contains(const MusicInfo &info);
    MusicList allmusic();

public slots:
    void setDisplayName(const QString &name);
    void appendMusic(const MusicInfo &info);
    void removeMusic(const MusicInfo &info);

    //! private interface
public:
    void load();
    void save();

signals:
    void musicAdded(const MusicInfo &info);
    void musicRemoved(const MusicInfo &info);
    void removed();
    void displayNameChanged(QString displayName);

private:
    QSettings       settings;
    MusicListInfo   listinfo;
};

typedef QSharedPointer<Playlist> PlaylistPtr;

extern const QString AllMusicListID;
extern const QString FavMusicListID;
extern const QString SearchMusicListID;

#endif // PLAYLIST_H
