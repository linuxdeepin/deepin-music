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

#include <dcombobox.h>
class Playlist;
class MusicItem;
class MusicMeta;
class MusicListView;

class MusicListWidget : public QFrame
{
    Q_OBJECT
public:
    explicit MusicListWidget(QWidget *parent = 0);

    void setCurrentList(QSharedPointer<Playlist> palylist);
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

signals:
    void musicRemove(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void musicClicked(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void musicAdd(const QString &playlistID, const MusicMeta &info);
    void playall(QSharedPointer<Playlist> palylist);
    void requestCustomContextMenu(MusicItem* item, const QPoint &pos);
    void resort(QSharedPointer<Playlist> palylist, int sortType);

public slots:
    void onMusicPlayed(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void onMusicRemoved(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void onMusicAdded(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void onMusiclistChanged(QSharedPointer<Playlist> palylist);
    void onCustomContextMenuRequest(MusicItem* item,
                                    const QPoint &pos,
                                    QSharedPointer<Playlist> selectedlist,
                                    QSharedPointer<Playlist> favlist,
                                    QList<QSharedPointer<Playlist> >newlists);

signals:
    void showEmpty(bool isEmpty);

private:
    void addMusicInfo(MusicListView *m_musiclist, const MusicMeta &info);

    QSharedPointer<Playlist>    m_palylist;
    MusicItem                   *m_last         = nullptr;
    MusicListView               *m_musiclist    = nullptr;
    Dtk::Widget::DComboBox      *m_sortCombo    = nullptr;
};


#endif // MUSICLISTWIDGET_H
