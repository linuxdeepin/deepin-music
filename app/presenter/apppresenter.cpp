/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "apppresenter.h"

#include <QDebug>
#include <QDir>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QUrl>
#include <QThread>

#include <taglib/tag.h>
#include <fileref.h>

#include "../core/playlistmanager.h"
#include <QApplication>
class AppPresenterPrivate
{
public:
    QMediaPlayer    player;
    PlaylistManager playlistMgr;
};

AppPresenter::AppPresenter(QObject *parent)
    : QObject(parent), d(new AppPresenterPrivate)
{
    qRegisterMetaType<MusicListInfo>();
    qRegisterMetaType<MusicInfo>();
}

AppPresenter::~AppPresenter()
{

}

QMediaPlayer *AppPresenter::player()
{
    return &d->player;
}

void AppPresenter::work()
{
    MusicListInfo model;

    // TODO: test data
    for (int i = 0; i < 20; ++i) {
        MusicInfo info;
        info.title = "Adrian Benson";
        info.artist = "Georgie Boone";
        info.album = "Patrick Alvarez";
        info.lenght = 300;
        model.list.append(info);
    }

    emit musicListChanged(model);
}

void AppPresenter::onMusicPlay(const MusicInfo &info)
{
    d->player.setMedia(QUrl::fromLocalFile(info.url));

    connect(&d->player, &QMediaPlayer::mediaStatusChanged,
    this, [ = ](QMediaPlayer::MediaStatus status) {
        qDebug() << status << info.url;
        d->player.play();
        emit musicPlayed(info);
    });
}

void AppPresenter::onFilesImportDefault(const QStringList &filelist)
{
    MusicListInfo model;

    qDebug() << QThread::currentThread() << qApp->thread();

    QStringList urllist;
    // save default playlist
    for (auto &dir : filelist) {
        for (auto fileinfo : QDir(dir).entryInfoList()) {
            //TODO: check file type
            if (fileinfo.fileName().endsWith("mp3")) {
                urllist.append(fileinfo.absoluteFilePath());
            }
        }
    }

    if (urllist.empty()) {
        qCritical() << "can not find meida file";
        return;
    }

    emit musicListChanged(model);

    for (auto &url : urllist) {
        TagLib::FileRef f(url.toStdString().c_str());

        MusicInfo info;
        info.url = url;
        info.title = QString::fromUtf8(f.tag()->title().toCString(true));
        info.artist = QString::fromUtf8(f.tag()->artist().toCString(true));
        info.album = QString::fromUtf8(f.tag()->album().toCString(true));
        info.lenght = f.audioProperties()->length();

        if (info.title.isEmpty()) {
            info.title = QFileInfo(url).baseName();
        }

        if (info.artist.isEmpty()) {
            info.artist = tr("Unknow Artist");
        }

        if (info.album.isEmpty()) {
            info.album = tr("Unknow Album");
        }

        emit musicAdded(info);
    }
}
