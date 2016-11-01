/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MEDIAFILEMONITOR_H
#define MEDIAFILEMONITOR_H

#include <QObject>
#include <QSharedPointer>

#include "playlist.h"

class MediaFileMonitor : public QObject
{
    Q_OBJECT
public:
    static QStringList supportedFilterStringList();

    explicit MediaFileMonitor(QObject *parent = 0);

signals:
    void meidaFileImported(QSharedPointer<Playlist> playlist, MusicMetaList metalist);

public slots:
    void importPlaylistFiles(QSharedPointer<Playlist> playlist, const QStringList &filelist);

};

#endif // MEDIAFILEMONITOR_H
