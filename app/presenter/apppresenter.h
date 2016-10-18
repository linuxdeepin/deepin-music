/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef APPPRESENTER_H
#define APPPRESENTER_H

#include <QObject>
#include <QScopedPointer>

#include "model/musiclistmodel.h"

class AppPresenterPrivate;
class QMediaPlayer;
class AppPresenter : public QObject
{
    Q_OBJECT
public:
    explicit AppPresenter(QObject *parent = 0);
    ~AppPresenter();
    QMediaPlayer *player();
    void work();

signals:
    void musicListChanged(const MusicListInfo &listinfo);
    void musicAdded(const MusicInfo &info);
    void musicPlayed(const MusicInfo &info);

public slots:
    void onMusicPlay(const MusicInfo &info);
    void onFilesImportDefault(const QStringList &filelist);

private:
    void prepareData();
    void loadUrlList(QStringList urllist, QMediaPlayer *player);

    QScopedPointer<AppPresenterPrivate> d;
};

#endif // APPPRESENTER_H
