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

class MusicInfo;
class Playlist;
class FooterPrivate;
class Footer : public QFrame
{
    Q_OBJECT
public:
    explicit Footer(QWidget *parent = 0);

signals:
    void initFooter(QSharedPointer<Playlist> favlist, int mode);
    void play(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void pause(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void next(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void prev(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void showLyric();
    void changePlayMode(int);

public slots:
    void onMusicPlay(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void onMusicPause(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void onMusicStop(QSharedPointer<Playlist> palylist, const MusicInfo &info);

private:
    void updateQssProperty(QWidget *w, const char *name, const QVariant &value);

    QSharedPointer<FooterPrivate>     d;
};


#endif // FOOTER_H
