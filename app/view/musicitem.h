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
class MusicMeta;
class Playlist;
class MusicItem : public QWidget
{
    Q_OBJECT
public:
    explicit MusicItem(int num, const MusicMeta &info, QWidget *parent = 0);

    const MusicMeta &info() {return m_info;}
signals:
    void play();
    void stop();
    void remove();
    void deleteFile();
    void addToPlaylist(const QString &id);
    void requestCustomContextMenu(const QPoint &pos);

public slots:
    void onMusicPlay();
    void onMusicStop();

    void showContextMenu(const QPoint &pos,
                         QSharedPointer<Playlist> selectedlist,
                         QSharedPointer<Playlist> favlist,
                         QList<QSharedPointer<Playlist> >newlist);
private:
    MusicMeta   m_info;
    QLabel      *m_number = nullptr;
};

#endif // MUSICITEM_H
