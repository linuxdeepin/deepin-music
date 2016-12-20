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

#include "../core/playlist.h"

class MusicListWidgetPrivate;
class MusicListWidget : public QFrame
{
    Q_OBJECT
public:
    explicit MusicListWidget(QWidget *parent = 0);
    ~MusicListWidget();

    void initData(PlaylistPtr playlist);

public slots:
    void onMusicPlayed(PlaylistPtr playlist, const MusicMeta &info);
    void onMusicRemoved(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicAdded(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicListAdded(PlaylistPtr playlist, const MusicMetaList &infolist);

    void onLocate(PlaylistPtr playlist, const MusicMeta &info);

    void onMusiclistChanged(PlaylistPtr playlist);
    void onCustomContextMenuRequest(const QPoint &pos,
                                    PlaylistPtr selectedlist,
                                    PlaylistPtr favlist,
                                    QList<PlaylistPtr >newlists);

protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<MusicListWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusicListWidget)
};

