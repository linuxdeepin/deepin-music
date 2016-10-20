/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "apppresenter.h"

#include <QDir>
#include <QUrl>
#include <QThread>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QCryptographicHash>
#include <QApplication>
#include <QDebug>

#include <taglib/tag.h>
#include <fileref.h>

#include "../core/playlistmanager.h"

class AppPresenterPrivate
{
public:
    QMediaPlayer    *player;
    PlaylistManager playlistMgr;
};

AppPresenter::AppPresenter(QObject *parent)
    : QObject(parent), d(new AppPresenterPrivate)
{
    qRegisterMetaType<MusicListInfo>();
    qRegisterMetaType<MusicInfo>();
    qRegisterMetaType<QSharedPointer<Playlist> >();

    d->player = new QMediaPlayer(this);
    //! load playlist
    d->playlistMgr.load();

    connect(this, &AppPresenter::musicAdded,
    this, [ = ](QSharedPointer<Playlist> palylist, const MusicInfo & info) {
        if (palylist.isNull()) {
            qWarning() << "add music to empty playlist." << info.url;
            return;
        }
        palylist->appendMusic(info);
    });

    connect(&d->playlistMgr, &PlaylistManager::currentPlaylistChanged,
    this, [ = ](QSharedPointer<Playlist> playlist) {
        emit playlistChanged(playlist);
    });
}

AppPresenter::~AppPresenter()
{

}

QSharedPointer<Playlist> AppPresenter::allMusicPlaylist()
{
    return d->playlistMgr.playlist(AllMusicListID);
}

QSharedPointer<Playlist> AppPresenter::lastPlaylist()
{
    return d->playlistMgr.currentPlaylist();
}

QList<QSharedPointer<Playlist> > AppPresenter::allplaylist()
{
    return d->playlistMgr.allplaylist();
}

void AppPresenter::onMusicRemove(QSharedPointer<Playlist> playlist, const MusicInfo &info)
{
    qDebug() << "onMusicRemove";
    playlist->removeMusic(info);

    // TODO: do better;
    if (playlist->id() == AllMusicListID) {
        for (auto &playlist : allplaylist()) {
            if (playlist->contains(info)) {
                playlist->removeMusic(info);
            }
        }
    }
}

void AppPresenter::onMusicAdd(const QString &id, const MusicInfo &info)
{
    QString listid = id;
    if (id == "New") {
        MusicListInfo info;
        info.editmode = false;
        info.readonly = false;
        info.id = d->playlistMgr.newID();
        info.displayName = d->playlistMgr.newDisplayName();
        qDebug() << "get new " << info.displayName;
        d->playlistMgr.addPlaylist(info);

        emit playlistAdded(d->playlistMgr.playlist(info.id));
        listid = info.id;
    }
    if (d->playlistMgr.playlist(listid).isNull()) {
        qDebug() << "no list";
        return;
    }
    d->playlistMgr.playlist(listid)->appendMusic(info);
}

void AppPresenter::onPlaylistChange(QSharedPointer<Playlist> playlist)
{
    d->playlistMgr.setCurrentPlaylist(playlist);
}

void AppPresenter::onPlaylistAdd(bool edit)
{
    MusicListInfo info;
    info.editmode = edit;
    info.readonly = false;
    info.id = d->playlistMgr.newID();
    info.displayName = d->playlistMgr.newDisplayName();
    d->playlistMgr.addPlaylist(info);

    emit playlistAdded(d->playlistMgr.playlist(info.id));
}

void AppPresenter::onMusicPlay(QSharedPointer<Playlist> palylist, const MusicInfo &info)
{
    d->player->setMedia(QUrl::fromLocalFile(info.url));

    connect(d->player, &QMediaPlayer::mediaStatusChanged,
    this, [ = ](QMediaPlayer::MediaStatus status) {
        qDebug() << status << info.url;
        if (QMediaPlayer::PlayingState != d->player->state()) {
            d->player->play();
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
    QSharedPointer<Playlist> playlist = d->playlistMgr.playlist(AllMusicListID);

    emit showMusiclist();
    QThread::msleep(50);
    bool listshow = false;
    for (auto &url : urllist) {
        TagLib::FileRef f(url.toStdString().c_str());

        MusicInfo info;
        info.url = url;
        info.id = QString(QCryptographicHash::hash(info.url.toUtf8(), QCryptographicHash::Md5).toHex());
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

        qDebug() << "send" << info.title;
        emit musicAdded(playlist, info);
        if (!listshow) {
        }
        listshow = true;
    }

}
