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

class MusicMeta;
class Playlist;
class FooterPrivate;
class Footer : public QFrame
{
    Q_OBJECT
public:
    explicit Footer(QWidget *parent = 0);

    void enableControl(bool enable=true);

signals:
    void initFooter(QSharedPointer<Playlist> favlist, QSharedPointer<Playlist> current, int mode);

    void changeProgress(qint64 value, qint64 duration);

    void play(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void pause(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void next(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void prev(QSharedPointer<Playlist> palylist, const MusicMeta &info);

    void modeChanged(int mode);
    void toggleFavourite(const MusicMeta &info);
    void toggleLyric();
    void changePlayMode(int);
    void togglePlaylist();

public slots:
    void onMusicAdded(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void onMusicRemoved(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void onMusicPlay(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void onMusicPause(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void onMusicStop(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void onProgressChanged(qint64 value, qint64 duration);
    void onCoverChanged(const MusicMeta &info, const QString &coverPath);

private:
    void updateQssProperty(QWidget *w, const char *name, const QVariant &value);

    QSharedPointer<FooterPrivate>     d;
};

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(const QString &text = "", QWidget *parent = 0);
    ~ClickableLabel();
signals:
    void clicked(bool);
protected:
    void mousePressEvent(QMouseEvent *event);
};

#endif // FOOTER_H
