/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef FOOTER_H
#define FOOTER_H

#include <QFrame>
#include <QPointer>
#include <QLabel>

#include "../core/playlist.h"

class MusicMeta;
class Playlist;
class FooterPrivate;
class Footer : public QFrame
{
    Q_OBJECT
public:
    explicit Footer(QWidget *parent = 0);

public:
    void enableControl(bool enable = true);

signals:
    void initFooter(QSharedPointer<Playlist> current, int mode);

    void changeProgress(qint64 value, qint64 duration);

    void play(QSharedPointer<Playlist> playlist, const MusicMeta &meta);
    void resume(QSharedPointer<Playlist> playlist, const MusicMeta &meta);
    void pause(QSharedPointer<Playlist> playlist, const MusicMeta &meta);
    void next(QSharedPointer<Playlist> playlist, const MusicMeta &meta);
    void prev(QSharedPointer<Playlist> playlist, const MusicMeta &meta);
    void locate(QSharedPointer<Playlist> playlist, const MusicMeta &meta);

    void modeChanged(int mode);
    void toggleFavourite(const MusicMeta &meta);
    void toggleLyric();
    void changePlayMode(int);
    void togglePlaylist();

public slots:
    void onMusicAdded(QSharedPointer<Playlist> playlist, const MusicMeta &meta);
    void onMusicListAdded(QSharedPointer<Playlist> playlist, const MusicMetaList &metalist);
    void onMusicRemoved(QSharedPointer<Playlist> playlist, const MusicMeta &meta);
    void onMusicPlayed(QSharedPointer<Playlist> playlist, const MusicMeta &meta);
    void onMusicPause(QSharedPointer<Playlist> playlist, const MusicMeta &meta);
    void onMusicStoped(QSharedPointer<Playlist> playlist, const MusicMeta &meta);
    void onProgressChanged(qint64 value, qint64 duration);
    void onCoverChanged(const MusicMeta &meta, const QString &coverPath);

private:
    void updateQssProperty(QWidget *w, const char *name, const QVariant &value);

    QSharedPointer<FooterPrivate>     d;
};

#endif // FOOTER_H
