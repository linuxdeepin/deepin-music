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

#include "music.h"

class PlaylistMeta
{
public:
    PlaylistMeta(){}

    QString uuid;
    QString displayName;
    QString url;
    QString icon;

    int     sortType    = 0;
    bool    editmode    = false;
    bool    readonly    = false;
    bool    hide        = false;
    bool    active      = false;
    bool    unused      = false;

    MusicMeta                   playing;
    QStringList                 sortMetas;

    QMap<QString, MusicMeta>    metas;
    QMap<QString, int>          invalidMetas;
};

Q_DECLARE_METATYPE(PlaylistMeta);

class Playlist : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(bool active READ active WRITE setActive)
public:
    explicit Playlist(const PlaylistMeta &musiclistinfo, QObject *parent = 0);

    enum SortType {
        SortByAddTime  = 0,
        SortByTitle,
        SortByArtist,
        SortByAblum
    };
    Q_ENUM(SortType)

public:
    //! public interface
    QString id() const;
    QString displayName() const;
    QString icon() const;
    bool readonly() const;
    bool editmode() const;
    bool hide() const;
    bool isEmpty() const;
    bool canNext() const;
    int length() const;
    int sorttype() const;

    bool active() const;
    void setActive(bool active);

    const MusicMeta first() const;
    const MusicMeta prev(const MusicMeta &info) const;
    const MusicMeta next(const MusicMeta &info) const;
    const MusicMeta music(int index) const;
    const MusicMeta music(const QString &id) const;
    const MusicMeta playing() const;

    bool isLast(const MusicMeta &info) const;
    bool contains(const MusicMeta &info) const;
    MusicMetaList allmusic() const;

    void play(const MusicMeta &meta);
    void reset(const MusicMetaList &);

public slots:
    void setDisplayName(const QString &name);
    void appendMusic(const MusicMetaList &metalist);
    void updateMeta(const MusicMeta &meta);
    MusicMeta removeMusic(const MusicMetaList &metalist);
    MusicMeta removeOneMusic(const MusicMeta &meta);
    void sortBy(Playlist::SortType sortType);
    void resort();

public:
    void load();

signals:
    void musiclistAdded(const MusicMetaList &metalist);
    void musicAdded(const MusicMeta &info);
    void musicRemoved(const MusicMeta &info);
    void removed();
    void displayNameChanged(QString displayName);

private:
    Q_DISABLE_COPY(Playlist);
    PlaylistMeta   listmeta;
};

typedef QSharedPointer<Playlist> PlaylistPtr;

extern const QString AllMusicListID;
extern const QString FavMusicListID;
extern const QString SearchMusicListID;

#endif // PLAYLIST_H
