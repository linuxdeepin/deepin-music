/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MUSICLISTWIDGET_H
#define MUSICLISTWIDGET_H

#include <QFrame>

class Playlist;
class MusicInfo;
class MusicListInfo;
class QStringListModel;
class MusicListView;

class MusicListWidget : public QFrame
{
    Q_OBJECT
public:
    explicit MusicListWidget(QWidget *parent = 0);

    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

signals:
    void musicClicked(const MusicInfo &info);
    void musicAddToPlaylist(const QString &playlistID, const MusicInfo &info);

public slots:
    void onMusicAdded(const MusicInfo &info);
    void onMusicListChanged(QSharedPointer<Playlist> palylist);

private:
    void addMusicInfo(MusicListView *m_musiclist, const MusicInfo &info);
    QSharedPointer<Playlist>    m_palylist;
    MusicListView               *m_musiclist    = nullptr;
};


#endif // MUSICLISTWIDGET_H
