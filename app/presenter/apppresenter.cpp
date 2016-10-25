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
#include <QApplication>
#include <QStandardPaths>
#include <QDebug>

#include "../core/player.h"

#include "../core/playlistmanager.h"
#include "../core/mediafilemonitor.h"

class AppPresenterPrivate
{
public:
    PlaylistManager     playlistMgr;
    MediaFileMonitor     *moniter;
};

AppPresenter::AppPresenter(QObject *parent)
    : QObject(parent), d(new AppPresenterPrivate)
{
    qRegisterMetaType<MusicListInfo>();
    qRegisterMetaType<MusicInfo>();
    qRegisterMetaType<QSharedPointer<Playlist> >();
    qRegisterMetaType<QList<QSharedPointer<Playlist> > >();

    d->moniter = new MediaFileMonitor;
    auto work = new QThread;
    d->moniter->moveToThread(work);
    work->start();

    connect(this, &AppPresenter::importMediaFiles,
            d->moniter, &MediaFileMonitor::importPlaylistFiles);

    connect(d->moniter, &MediaFileMonitor::meidaFileImported,
    this, [ = ](QSharedPointer<Playlist> playlist, const MusicInfo & info) {
        if (playlist.isNull()) {
            qWarning() << "add music to empty playlist." << info.url;
            return;
        }
        playlist->appendMusic(info);
    });

    //! load playlist
    d->playlistMgr.load();

    connect(&d->playlistMgr, &PlaylistManager::playingPlaylistChanged,
    this, [ = ](QSharedPointer<Playlist> playlist) {
        emit playingPlaylistChanged(playlist);
    });

    connect(Player::instance(), &Player::progrossChanged, this, [ = ](qint64 position, qint64 duration) {
        emit progrossChanged(position, duration);
    });
    connect(this, &AppPresenter::setMedia, Player::instance(), &Player::setMedia);
    connect(this, &AppPresenter::play, Player::instance(), &QMediaPlayer::play);
    connect(this, &AppPresenter::pause, Player::instance(), &QMediaPlayer::pause);
    connect(this, &AppPresenter::stop, Player::instance(), &QMediaPlayer::stop);

    connect(&d->playlistMgr, &PlaylistManager::musicAdded,
            this, &AppPresenter::musicAdded);
    connect(&d->playlistMgr, &PlaylistManager::musicRemoved,
            this, &AppPresenter::musicRemoved);
    connect(&d->playlistMgr, &PlaylistManager::selectedPlaylistChanged,
            this, &AppPresenter::selectedPlaylistChanged);
}

AppPresenter::~AppPresenter()
{

}

QSharedPointer<Playlist> AppPresenter::allMusicPlaylist()
{
    return d->playlistMgr.playlist(AllMusicListID);
}

QSharedPointer<Playlist> AppPresenter::favMusicPlaylist()
{
    return d->playlistMgr.playlist(FavMusicListID);
}

QSharedPointer<Playlist> AppPresenter::lastPlaylist()
{
    return d->playlistMgr.playingPlaylist();
}

QList<QSharedPointer<Playlist> > AppPresenter::allplaylist()
{
    return d->playlistMgr.allplaylist();
}

int AppPresenter::playMode()
{
    return d->playlistMgr.playMode();
}

void AppPresenter::onMusicRemove(QSharedPointer<Playlist> playlist, const MusicInfo &info)
{
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
        emit showPlaylist();

        MusicListInfo info;
        info.editmode = true;
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

void AppPresenter::onSelectedPlaylistChanged(QSharedPointer<Playlist> playlist)
{
    d->playlistMgr.setSelectedPlaylist(playlist);
}

void AppPresenter::onRequestMusiclistMenu(MusicItem *item, const QPoint &pos)
{
    qDebug() << item;
    QList<QSharedPointer<Playlist> > newlists = d->playlistMgr.allplaylist();
    // remove all and fav and search
    newlists.removeAll(d->playlistMgr.playlist(AllMusicListID));
    newlists.removeAll(d->playlistMgr.playlist(FavMusicListID));
    newlists.removeAll(d->playlistMgr.playlist(SearchMusicListID));

    auto selectedlist = d->playlistMgr.selectedPlaylist();
    auto favlist = d->playlistMgr.playlist(FavMusicListID);

    emit this->musiclistMenuRequested(item, pos, selectedlist, favlist, newlists);
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

void AppPresenter::onMusicPlay(QSharedPointer<Playlist> palylist,  const MusicInfo &info)
{
    if (0 == d->playlistMgr.playlist(AllMusicListID)->length()) {
        emit requestImportFiles();
        return;
    }

    if (Player::instance()->media().canonicalUrl() == QUrl::fromLocalFile(info.url)) {
        emit this->play();
        return;
    }
    qDebug() << "Fix me: play status" << info.id  << info.url << Player::instance();
    emit this->setMedia(QUrl::fromLocalFile(info.url).toString());
    emit this->play();

    MusicInfo favInfo(info);
    favInfo.favourite = d->playlistMgr.playlist(FavMusicListID)->contains(info);
    emit musicPlayed(palylist, favInfo);
}

void AppPresenter::onMusicPause(QSharedPointer<Playlist> playlist, const MusicInfo &info)
{
    emit this->pause();
    emit musicPaused(playlist, playlist->next(info));
}

void AppPresenter::onMusicPrev(QSharedPointer<Playlist> playlist, const MusicInfo &info)
{
    onMusicPlay(playlist, playlist->prev(info));
}

void AppPresenter::onMusicNext(QSharedPointer<Playlist> playlist, const MusicInfo &info)
{
    onMusicPlay(playlist, playlist->next(info));
}

void AppPresenter::onToggleFavourite(const MusicInfo &info)
{
    if (d->playlistMgr.playlist(FavMusicListID)->contains(info)) {
        d->playlistMgr.playlist(FavMusicListID)->removeMusic(info);
    } else {
        d->playlistMgr.playlist(FavMusicListID)->appendMusic(info);
    }
}

void AppPresenter::onChangeProgress(qint64 value, qint64 range)
{
    auto position = value * Player::instance()->duration() / range;
    if (position < 0) {
        qCritical() << "invaild position:" << Player::instance()->media().canonicalUrl() << position;
    }
    Player::instance()->setPosition(position);
}

void AppPresenter::onPlayall(QSharedPointer<Playlist> playlist)
{
    this->onMusicPlay(playlist, playlist->first());
}

void AppPresenter::onImportFiles(const QStringList &filelist)
{
    QSharedPointer<Playlist> playlist = d->playlistMgr.playlist(AllMusicListID);
    emit importMediaFiles(playlist, filelist);
    emit showMusiclist();
    return;
    /*
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
        info.length = f.audioProperties()->length();

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
    }*/

}

void AppPresenter::onImportMusicDirectory()
{
    auto musicDir =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    qDebug() << "scan" << musicDir;
    onImportFiles(musicDir);
}

