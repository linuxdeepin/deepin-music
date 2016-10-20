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

#include "../core/playlist.h"

class Footer : public QFrame
{
    Q_OBJECT
public:
    explicit Footer(QWidget *parent = 0);

signals:
    void initFooter(QSharedPointer<Playlist> favlist, int mode);
    void play();
    void pasue();
    void next();
    void prev();
    void showLyric();
    void changePlayMode(int);

public slots:
    void onMusicPause(const MusicInfo &info);
    void onMusicStop(const MusicInfo &info);

private:
    MusicInfo                   m_info;
    int                         m_mode;
    QSharedPointer<Playlist>    m_favlist;
};


#endif // FOOTER_H
