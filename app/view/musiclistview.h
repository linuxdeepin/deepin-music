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
class QScrollBar;
class MusicListView : public QTableView
{
    Q_OBJECT
    Q_PROPERTY(QColor headerColor READ headerColor WRITE setHeaderColor NOTIFY headerColorChanged)
public:
    explicit MusicListView(QWidget *parent = 0);


    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

    QStandardItemModel *model() {return m_model;}

    QColor headerColor() const
    {
        return m_headerColor;
    }

signals:
    void play(const MusicMeta &meta);
    void addToPlaylist(QSharedPointer<Playlist> playlist, const MusicMetaList metalist);
    void removeMusicList(const MusicMetaList &metalist);
    void deleteMusicList(const MusicMetaList &metalist);
    void requestCustomContextMenu(const QPoint &pos);

    void headerColorChanged(QColor headerColor);

public slots:
    void showContextMenu(const QPoint &pos,
                         QSharedPointer<Playlist> selectedlist,
                         QSharedPointer<Playlist> favlist,
                         QList<QSharedPointer<Playlist> >newlist);

    void setHeaderColor(QColor headerColor)
    {
        if (m_headerColor == headerColor) {
            return;
        }

        m_headerColor = headerColor;
        emit headerColorChanged(headerColor);
    }

private:
    QStandardItemModel  *m_model = nullptr;
    QScrollBar          *m_scrollBar = nullptr;
    QColor m_headerColor;
};

#endif // MUSICLISTVIEW_H
