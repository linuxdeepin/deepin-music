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
    qRegisterMetaType<QSharedPointer<Playlist> >();

    d->playlistMgr.load();

    connect(this, &AppPresenter::musicAdded,
    this, [ = ](const MusicInfo & info) {
        d->playlistMgr.currentPlaylist()->addMusic(info);
    });

    connect(&d->playlistMgr, &PlaylistManager::currentPlaylistChanged,
    this, [ = ](QSharedPointer<Playlist> playlist) {
        emit musicListChanged(playlist);
    });
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
}

QSharedPointer<Playlist> AppPresenter::lastPlaylist()
{
    return d->playlistMgr.currentPlaylist();
}

QList<QSharedPointer<Playlist> > AppPresenter::playlist()
{
    return d->playlistMgr.allplaylist();
}

void AppPresenter::onMusicAddToplaylist(const QString &id, const MusicInfo &info)
{
    QString listid = id;
    if (id == "New") {
        MusicListInfo info;
        info.editmode = false;
        info.readonly = false;
        info.id = d->playlistMgr.newID();
        info.displayName = d->playlistMgr.newDisplayName();
        d->playlistMgr.addplaylist(info);

        emit playlistAdded(d->playlistMgr.playlist(info.id));
        listid = info.id;
    }
    if (d->playlistMgr.playlist(listid).isNull()) {
        qDebug() << "no list";
        return;
    }
    d->playlistMgr.playlist(listid)->addMusic(info);
}

void AppPresenter::onPlaylistSelected(QSharedPointer<Playlist> playlist)
{
    // TODO: write current list
    qDebug() << playlist;
    d->playlistMgr.setCurrentPlaylist(playlist);
}

void AppPresenter::onPlaylistAdd(bool edit)
{
    MusicListInfo info;
    info.editmode = edit;
    info.readonly = false;
    info.id = d->playlistMgr.newID();
    info.displayName = d->playlistMgr.newDisplayName();
    d->playlistMgr.addplaylist(info);

    emit playlistAdded(d->playlistMgr.playlist(info.id));
}

void AppPresenter::onMusicPlay(const MusicInfo &info)
{
    d->player.setMedia(QUrl::fromLocalFile(info.url));

    connect(&d->player, &QMediaPlayer::mediaStatusChanged,
    this, [ = ](QMediaPlayer::MediaStatus status) {
        qDebug() << status << info.url;
        if (QMediaPlayer::PlayingState != d->player.state()) {
            d->player.play();
        }
        emit musicPlayed(info);
    });
}

void AppPresenter::onFilesImportDefault(const QStringList &filelist)
{
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
    bool listshow = false;
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
        if (!listshow) {
            emit showMusiclist();
        }
        listshow = true;
    }

}
