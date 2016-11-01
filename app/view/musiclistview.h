/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MUSICLISTVIEW_H
#define MUSICLISTVIEW_H

#include <QTableView>

#include "../model/musiclistmodel.h"

class Playlist;
class QStandardItemModel;
class MusicListView : public QTableView
{
    Q_OBJECT
public:
    explicit MusicListView(QWidget *parent = 0);

    QStandardItemModel *model() {return m_model;}

signals:
    void play(const MusicMeta &meta);
    void addToPlaylist(QSharedPointer<Playlist> playlist, const MusicMetaList metalist);
    void remove(const MusicMeta &meta);
    void requestCustomContextMenu(const QPoint &pos);

public slots:
    void showContextMenu(const QPoint &pos,
                         QSharedPointer<Playlist> selectedlist,
                         QSharedPointer<Playlist> favlist,
                         QList<QSharedPointer<Playlist> >newlist);
private:
    QStandardItemModel *m_model = nullptr;
};

#endif // MUSICLISTVIEW_H
