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

class MusicInfo;
class QNetworkReply;
class LyricService : public QObject
{
    Q_OBJECT
public:
    explicit LyricService(QObject *parent = 0);

signals:
    void lyricSearchFinished(const MusicInfo &, const QString &lyricPath);
    void coverSearchFinished(const MusicInfo &, const QString &coverPath);

public slots:
    void searchLyricCover(const MusicInfo &info);

private:
    int doSongArtistRequest(const MusicInfo &info);
    int doSongRequest(const MusicInfo &info);
    int doLyricRequest(const MusicInfo &info, int sid);
    int doCoverRequest(const MusicInfo &info, int aid);

    int downloadUrl(const QString &url, const QString &filepath);
};

#endif // LYRICSERVICE_H
