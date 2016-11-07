/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef LYRICSERVICE_H
#define LYRICSERVICE_H

#include <QObject>

class MusicMeta;
class QNetworkReply;
class LyricService : public QObject
{
    Q_OBJECT
public:
    explicit LyricService(QObject *parent = 0);

    static QString coverPath(const MusicMeta &info);
signals:
    void lyricSearchFinished(const MusicMeta &, const QString &lyricPath);
    void coverSearchFinished(const MusicMeta &, const QString &coverPath);

public slots:
    void searchLyricCover(const MusicMeta &info);

private:
    int searchCacheLyric(const MusicMeta &info);
    int searchCacheCover(const MusicMeta &info);

    int doSongArtistRequest(const MusicMeta &info, bool lyric, bool cover);
    int doSongRequest(const MusicMeta &info, bool lyric, bool cover);
    int doLyricRequest(const MusicMeta &info, int sid);
    int doCoverRequest(const MusicMeta &info, int aid);

    int downloadUrl(const QString &url, const QString &filepath);
};

#endif // LYRICSERVICE_H
