///**
// * Copyright (C) 2016 Deepin Technology Co., Ltd.
// *
// * This program is free software; you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation; either version 3 of the License, or
// * (at your option) any later version.
// **/

//#pragma once

//#include <QSharedPointer>
//#include <QMediaPlayer>

//#include "music.h"
//#include "playlist.h"

//class QPlayer : public QMediaPlayer
//{
//    Q_OBJECT
//    Q_PROPERTY(PlayMode mode READ mode WRITE setMode NOTIFY modeChanged)
//public:

//    enum PlayStatus {
//        Invaild = 0,
//        Stop,
//        Playing,
//        Pause
//    };
//    Q_ENUM(PlayStatus)

//    enum PlayMode {
//        RepeatAll = 0,
//        RepeatSingle = 1,
//        Shuffle = 2,
//    };
//    Q_ENUM(PlayMode)

//    static QPlayer *instance()
//    {
//        static auto s_app = new QPlayer;
//        return s_app;
//    }

//    QStringList supportedFilterStringList();
//    QStringList supportedMimeTypes();

//    const MusicMeta playingMeta();

//    void setPlaylist(PlaylistPtr);
//    void setMode(PlayMode mode);

//    void init() {}

//    inline PlayMode mode() const { return m_mode; }

//signals:
//    void progrossChanged(qint64 value, qint64 range);
//    void modeChanged(PlayMode mode);
//    void musicPlayed(PlaylistPtr playlist, const MusicMeta &meta);

//public slots:
//    void playMusic(PlaylistPtr playlist, const MusicMeta &meta);
//    void resumeMusic(PlaylistPtr playlist, const MusicMeta &meta);
//    void playNextMusic(PlaylistPtr playlist, const MusicMeta &meta);
//    void playPrevMusic(PlaylistPtr playlist, const MusicMeta &meta);
//    void changeProgress(qint64 value, qint64 range);

//private:
//    ~QPlayer();
//    void setMediaMeta(const MusicMeta &meta);
//    void selectNext(const MusicMeta &meta, PlayMode mode);
//    void selectPrev(const MusicMeta &meta, PlayMode mode);
//    explicit QPlayer(QObject *parent = 0);

//    MusicMeta                   m_playingMeta;
//    QStringList                 m_historyIDs;
//    PlaylistPtr    m_playinglist;
//    QMap<QString, MusicMeta>    musicMap;
//    qint64                      m_duration = -1;
//    PlayMode                    m_mode;
//};
