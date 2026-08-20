// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Minimal in-process PlayerBase subclass used to (1) exercise PlayerBase's
// non-pure methods and (2) inject a fake backend into PlayerEngine so its tests
// never touch real VLC/SDL/Qt multimedia playback.
#pragma once
#include "player/playerbase.h"

class FakePlayer : public PlayerBase {
public:
    explicit FakePlayer(QObject *parent = nullptr) : PlayerBase(parent) { m_supportedSuffix << "mp3" << "flac" << "wav"; }
    void init() override {}
    void release() override {}
    DmGlobal::PlaybackStatus state() override { return m_state; }
    void setState(DmGlobal::PlaybackStatus s) { m_state = s; }
    void play() override {}
    void pause() override {}
    void stop() override {}
    int length() override { return 0; }
    void setTime(qint64) override {}
    qint64 time() override { return 0; }
    void setVolume(int volume) override { m_volume = volume; }
    int getVolume() override { return m_volume; }
    void setMute(bool) override {}
    bool getMute() override { return false; }
    void setMediaMeta(MediaMeta meta) override { m_activeMeta = meta; }
    void setFadeInOutFactor(double) override {}

    // Emit helpers so tests can drive PlayerEngine's signal-connected lambdas
    // (stateChanged/timeChanged/positionChanged/metaChanged handlers).
    void emitMetaChanged() { emit metaChanged(); }
    void emitTimeChanged(qint64 t) { emit timeChanged(t); }
    void emitPositionChanged(float p) { emit positionChanged(p); }
    void emitStateChanged(DmGlobal::PlaybackStatus s) { emit stateChanged(s); }

    DmGlobal::PlaybackStatus m_state = DmGlobal::Stopped;
};
