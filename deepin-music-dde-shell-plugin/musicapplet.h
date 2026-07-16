// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTimer>
#include <QUrl>

class MusicProber;

class MusicApplet : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool musicAvailable READ musicAvailable NOTIFY musicStateChanged FINAL)
    Q_PROPERTY(QString titleText READ titleText NOTIFY musicStateChanged FINAL)
    Q_PROPERTY(QString subtitleText READ subtitleText NOTIFY musicStateChanged FINAL)
    Q_PROPERTY(QString appName READ appName NOTIFY musicStateChanged FINAL)
    Q_PROPERTY(QUrl artSource READ artSource NOTIFY musicStateChanged FINAL)
    Q_PROPERTY(bool musicPlaying READ musicPlaying NOTIFY musicStateChanged FINAL)
    Q_PROPERTY(bool canGoPrevious READ canGoPrevious NOTIFY musicStateChanged FINAL)
    Q_PROPERTY(bool canGoNext READ canGoNext NOTIFY musicStateChanged FINAL)
    Q_PROPERTY(bool canTogglePlayback READ canTogglePlayback NOTIFY musicStateChanged FINAL)
    // Changes when the system icon theme changes; QML appends it to image://theme
    // URLs to bust QtQuick's image cache and force a reload from the new theme.
    Q_PROPERTY(QString iconThemeKey READ iconThemeKey NOTIFY iconThemeKeyChanged FINAL)

public:
    explicit MusicApplet(QObject *parent = nullptr);
    ~MusicApplet() override;

    bool musicAvailable() const;
    QString titleText() const;
    QString subtitleText() const;
    QString appName() const;
    QUrl artSource() const;
    bool musicPlaying() const;
    bool canGoPrevious() const;
    bool canGoNext() const;
    bool canTogglePlayback() const;
    QString iconThemeKey() const;

    Q_INVOKABLE void openMusicPlayer();
    Q_INVOKABLE void playPreviousTrack();
    Q_INVOKABLE void toggleMusicPlayback();
    Q_INVOKABLE void playNextTrack();

signals:
    void musicStateChanged();
    void iconThemeKeyChanged();

private slots:
    void refreshMusicState();
    void markStateDirty();
    void onAppearancePropertiesChanged(const QString &interfaceName,
                                       const QVariantMap &changedProperties,
                                       const QStringList &invalidatedProperties);

private:
    MusicProber *m_prober = nullptr;

    bool m_musicAvailable = false;
    QString m_titleText = QStringLiteral("未检测到音乐");
    QString m_subtitleText = QStringLiteral("打开播放器开始播放");
    QString m_appName = QStringLiteral("音乐");
    QUrl m_artSource;
    bool m_musicPlaying = false;
    bool m_canGoPrevious = false;
    bool m_canGoNext = false;
    bool m_canTogglePlayback = false;

    QString m_iconThemeKey;

    bool m_stateDirty = true;

    QTimer *m_debounceTimer = nullptr;
};
