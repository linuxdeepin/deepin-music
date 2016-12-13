/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef LYRICVIEW_H
#define LYRICVIEW_H

#include <QFrame>
#include <QScopedPointer>

#include "../core/music.h"
#include "../core/playlist.h"

class LyricViewPrivate;
class LyricView : public QFrame
{
    Q_OBJECT
public:
    explicit LyricView(QWidget *parent = 0);
    ~LyricView();

    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

    void setLyricLines(QString lines);

signals:
    void hideLyricView();

public slots:
    void onMusicPlayed(PlaylistPtr playlist, const MusicMeta &meta);
    void onProgressChanged(qint64 value, qint64 length);
    void onLyricChanged(const MusicMeta &meta, const QString &lyricPath);
    void onCoverChanged(const MusicMeta &meta, const QString &coverPath);

private:
    void initConnection();
    void adjustLyric();
    QScopedPointer<LyricViewPrivate>  d;
};

#endif // LYRICVIEW_H
