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

class MusicInfo;

class Footer : public QFrame
{
    Q_OBJECT
public:
    explicit Footer(QWidget *parent = 0);

signals:

public slots:
    void onMusicPlay(const MusicInfo &info);
//    void onMusicPause(const MusicInfo &info);
//    void onMusicStop(const MusicInfo &info);
};

#endif // FOOTER_H
