/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MUSICITEM_H
#define MUSICITEM_H

#include <QWidget>

#include "../model/musiclistmodel.h"

class QLabel;
class MusicInfo;
class MusicItem : public QWidget
{
    Q_OBJECT
public:
    explicit MusicItem(int num, const MusicInfo &info, QWidget *parent = 0);

    const MusicInfo &info() {return m_info;}
signals:
    void play();
    void stop();
    void remove();
    void deleteFile();
    void addToPlaylist(const QString &id);

public slots:
    void onMusicPlay();
    void onMusicStop();

    void showContextMenu(const QPoint &pos);
private:
    MusicInfo   m_info;
    QLabel      *number = nullptr;
};

#endif // MUSICITEM_H
