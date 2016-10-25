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
class MusicItem;
class MusicInfo;
class MusicListView;

class MusicListWidget : public QFrame
{
    Q_OBJECT
public:
    explicit MusicListWidget(QWidget *parent = 0);

    void setCurrentList(QSharedPointer<Playlist> palylist);
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

signals:
    void musicRemove(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void musicClicked(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void musicAdd(const QString &playlistID, const MusicInfo &info);
    void playall(QSharedPointer<Playlist> palylist);
    void requestCustomContextMenu(MusicItem* item, const QPoint &pos);

public slots:
    void onMusicPlayed(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void onMusicRemoved(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void onMusicAdded(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void onMusiclistChanged(QSharedPointer<Playlist> palylist);
    void onCustomContextMenuRequest(MusicItem* item,
                                    const QPoint &pos,
                                    QSharedPointer<Playlist> selectedlist,
                                    QSharedPointer<Playlist> favlist,
                                    QList<QSharedPointer<Playlist> >newlists);

private:
    void addMusicInfo(MusicListView *m_musiclist, const MusicInfo &info);

    QSharedPointer<Playlist>    m_palylist;
    MusicItem                   *m_last         = nullptr;
    MusicListView               *m_musiclist    = nullptr;
};


#endif // MUSICLISTWIDGET_H
