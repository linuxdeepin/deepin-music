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

class InotifyEngine;
class MediaFileMonitor : public QObject
{
    Q_OBJECT
public:
    static QStringList supportedFilterStringList();

    explicit MediaFileMonitor(QObject *parent = 0);

signals:
    void fileRemoved(const QString &filepath);
    void meidaFileImported(PlaylistPtr playlist, MusicMetaList metalist);

public slots:
    void importPlaylistFiles(PlaylistPtr playlist, const QStringList &filelist);
    void startMonitor();

private:
    const int ScanCacheSize = 200;
//    InotifyEngine  *m_watcher;
};

#endif // MEDIAFILEMONITOR_H
