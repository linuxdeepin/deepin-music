/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>
#include <QMap>
#include <QSharedPointer>

#include <playlistmeta.h>

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
        SortByAblum,
        SortByCustom,
    };
    Q_ENUM(SortType)

    enum OrderType {
        Ascending = 0,
        Descending,
    };
    Q_ENUM(OrderType)

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
    int sortType() const;
    uint sortID() const;

    bool active() const;
    void setActive(bool active);

    const MetaPtr first() const;
    const MetaPtr prev(const MetaPtr meta) const;
    const MetaPtr next(const MetaPtr info) const;
    const MetaPtr music(int index) const;
    const MetaPtr music(const QString &id) const;
    const MetaPtr playing() const;

    int index(const QString &hash);
    bool isLast(const MetaPtr meta) const;
    bool contains(const MetaPtr meta) const;
    MetaPtrList allmusic() const;

    void play(const MetaPtr meta);
    void reset(const MetaPtrList);

public slots:
    void setDisplayName(const QString &name);
    void appendMusicList(const MetaPtrList metalist);
    MetaPtr removeMusicList(const MetaPtrList metalist);
    MetaPtr removeOneMusic(const MetaPtr meta);
    void updateMeta(const MetaPtr meta);
    void sortBy(Playlist::SortType sortType);
    void resort();
    void saveSort(QMap<QString, int> hashIndexs);

public:
    void load();

signals:
    void musiclistAdded(const MetaPtrList metalist);
    void musiclistRemoved(const MetaPtrList metalist);
    void removed();
    void displayNameChanged(QString displayName);

private:
    Q_DISABLE_COPY(Playlist)

    PlaylistMeta   playlistMeta;
};

typedef QSharedPointer<Playlist> PlaylistPtr;

Q_DECLARE_METATYPE(PlaylistPtr)
Q_DECLARE_METATYPE(QList<PlaylistPtr>)

extern const QString AllMusicListID;
extern const QString FavMusicListID;
extern const QString SearchMusicListID;

