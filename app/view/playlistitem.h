/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QFrame>

class Playlist;
class QLineEdit;
class PlayListItem : public QFrame
{
    Q_OBJECT
public:
    explicit PlayListItem(QSharedPointer<Playlist> playlist, QWidget *parent = 0);
    inline QSharedPointer<Playlist> data() {return m_data;}

signals:
    void rename(const QString& newNameA);
    void remove();

public slots:
    void showContextMenu(const QPoint &pos);

private:
    QLineEdit                   *m_titleedit = nullptr;
    QSharedPointer<Playlist>    m_data;
};

#endif // PLAYLISTITEM_H
