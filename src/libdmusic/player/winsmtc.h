// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINSMTC_H
#define WINSMTC_H

#include <QObject>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wrl.h>
#include <wrl/wrappers/corewrappers.h>
#include <windows.media.h>
#include <systemmediatransportcontrolsinterop.h>

#define WINSMTC_AUMID L"Deepin.DeepinMusicPlayer"

class WinSMTC : public QObject
{
    Q_OBJECT
public:
    explicit WinSMTC(QObject *parent = nullptr);
    ~WinSMTC();

    bool initialize(HWND hwnd);
    void shutdown();

    static void ensureStartMenuShortcut();

    void updateMetadata(const QString &title,
                       const QString &artist,
                       const QString &album,
                       qint64 durationMs,
                       const QString &coverArtPath = QString());

    void updatePlaybackStatus(int status);

    void setControlsEnabled(bool play, bool pause, bool stop,
                           bool next, bool previous);

signals:
    void playRequested();
    void pauseRequested();
    void stopRequested();
    void nextRequested();
    void previousRequested();

private:
    Microsoft::WRL::ComPtr<ABI::Windows::Media::ISystemMediaTransportControls> m_smtc;
    Microsoft::WRL::ComPtr<ABI::Windows::Media::ISystemMediaTransportControlsDisplayUpdater> m_updater;
    Microsoft::WRL::ComPtr<ABI::Windows::Media::IMusicDisplayProperties> m_musicProps;
    bool m_initialized = false;
};

#endif // Q_OS_WIN

#endif // WINSMTC_H
