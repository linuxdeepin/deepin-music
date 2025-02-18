// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLAYERENGINE_H
#define PLAYERENGINE_H
#include <QObject>

#include "global.h"

class PlayerEnginePrivate;
class PlayerEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double fadeInOutFactor READ fadeInOutFactor WRITE setFadeInOutFactor NOTIFY fadeInOutFactorChanged)
public:
    PlayerEngine(QObject *parent = nullptr);
    ~PlayerEngine();

    double fadeInOutFactor() const;
    void setFadeInOut(bool flag);
    void setMprisPlayer(const QString &serviceName, const QString &desktopEntry, const QString &identity);
    void setMediaMeta(const QString &metaHash);
    void setMediaMeta(const DMusic::MediaMeta &metaList);
    QStringList supportedSuffixList()const;
    DMusic::MediaMeta getMediaMeta();
    void addMetasToPlayList(const QList<DMusic::MediaMeta> &metaList);
    void removeMetaFromPlayList(const QString &metaHash);
    void removeMetasFromPlayList(const QStringList &metaHashs);
    void clearPlayList(bool stopFlag = true);
    QList<DMusic::MediaMeta> getMetas();
    bool isEmpty();
    void playNextMeta(bool isAuto, bool playFlag = true);
    int length();
    void setTime(qint64 time);
    qint64 time();
    void setVolume(int volume);
    int getVolume();
    void setMute(bool mute);
    bool getMute();
    DmGlobal::PlaybackStatus playbackStatus();
    void setPlaybackMode(DmGlobal::PlaybackMode mode);
    DmGlobal::PlaybackMode getPlaybackMode();
    void setCurrentPlayList(const QString &playlistHash);
    QString getCurrentPlayList();
    QList<DMusic::MediaMeta> getCdaMetaInfo();

    // 均衡器
    void setEqualizerEnabled(bool enabled);
    void loadFromPreset(uint index);
    void setPreamplification(float value);
    void setAmplificationForBandAt(float amp, uint bandIndex);
    float amplificationForBandAt(uint bandIndex);
    float preamplification();

public slots:
    void play();
    void forcePlay();
    void pause();
    void pauseNow();
    void playPause();
    void resume();
    void playPreMeta();
    void stop();
    void setFadeInOutFactor(double fadeInOutFactor);

signals:
    void fadeInOutFactorChanged(double fadeInOutFactor);
    void metaChanged();
    void positionChanged(qint64 position, qint64 length);
    void playbackStatusChanged(DmGlobal::PlaybackStatus status);
    void muteChanged(bool mute);
    void volumeChanged(int volume);
    void openUriRequested(const QUrl &url);
    void playPictureChanged(const QString &path);
    void sendCdaStatus(int state);
    void quitRequested();
    void raiseRequested();
    void playPlaylistRequested(const QString &playlistHash);

private:
    void playNextMeta(const DMusic::MediaMeta &meta, bool isAuto, bool playFlag = true);
    void resetDBusMpris(const DMusic::MediaMeta &meta);

private:
    PlayerEnginePrivate  *m_data = nullptr;
};

#endif // PLAYERENGINE_H
