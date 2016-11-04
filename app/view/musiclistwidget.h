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

#include "../model/musiclistmodel.h"

class Playlist;
class MusicItem;
class MusicMeta;
class QLabel;
class MusicListView;
class QStandardItemModel;

class MusicListWidget : public QFrame
{
    Q_OBJECT
public:
    explicit MusicListWidget(QWidget *parent = 0);

    void setCurrentList(QSharedPointer<Playlist> playlist);
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

signals:
    void musicRemove(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void musicClicked(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void musicAdd(QSharedPointer<Playlist> playlist, const MusicMetaList &metalist);
    void playall(QSharedPointer<Playlist> playlist);
    void resort(QSharedPointer<Playlist> playlist, int sortType);
    void requestCustomContextMenu(const QPoint &pos);

public slots:
    void onMusicPlayed(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void onMusicRemoved(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void onMusicAdded(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void onMusicListAdded(QSharedPointer<Playlist> playlist, const MusicMetaList &infolist);

    void onLocate(QSharedPointer<Playlist> playlist, const MusicMeta &info);

    void onMusiclistChanged(QSharedPointer<Playlist> playlist);
    void onCustomContextMenuRequest(const QPoint &pos,
                                    QSharedPointer<Playlist> selectedlist,
                                    QSharedPointer<Playlist> favlist,
                                    QList<QSharedPointer<Playlist> >newlists);
signals:
    void showEmptyHits(bool empty);
private:
    void addMusicInfo(MusicListView *m_musiclist, const MusicMeta &info);

    QSharedPointer<Playlist>    m_playlist;
    QLabel                      *m_emptyHits    = nullptr;
    MusicListView               *m_musiclist    = nullptr;
    QStandardItemModel          *m_model        = nullptr;
    Dtk::Widget::DComboBox      *m_sortCombo    = nullptr;
};


#endif // MUSICLISTWIDGET_H
