/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QFrame>
#include <QPointer>
#include <QLabel>

#include "../core/playlist.h"

class FooterPrivate;
class Footer : public QFrame
{
    Q_OBJECT
public:
    explicit Footer(QWidget *parent = 0);
    ~Footer();

public:
    void enableControl(bool enable = true);
    void initData(PlaylistPtr current, int mode);

public slots:
    void onMusicAdded(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicListAdded(PlaylistPtr playlist, const MusicMetaList &metalist);
    void onMusicRemoved(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicPlayed(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicPause(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicStoped(PlaylistPtr playlist, const MusicMeta &meta);
    void onProgressChanged(qint64 value, qint64 duration);
    void onCoverChanged(const MusicMeta &meta, const QByteArray &coverData);

private:
    QScopedPointer<FooterPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Footer)
};

