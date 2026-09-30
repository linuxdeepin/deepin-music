// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMutex>
#include <QThread>
#include <QVector>
#include <QRandomGenerator>
#include <numeric>

class AudioDataDetector : public QThread
{
    Q_OBJECT
public:
    explicit AudioDataDetector(QObject *parent = Q_NULLPTR);
    ~AudioDataDetector();

public slots:
    void onBufferDetector(const QString &path, const QString &hash);
    void onClearBufferDetector();

signals:
    void audioBuffer(const QVector<float> &buffer, const QString &hash);
    void audioBufferFromThread(const QVector<float> &buffer, const QString &hash, quint64 requestGeneration);

private slots:
    void startPendingRequest();
    void forwardAudioBuffer(const QVector<float> &buffer, const QString &hash, quint64 requestGeneration);

private:
    void resample(const QVector<float> &buffer, const QString &hash,
                  quint64 requestGeneration, bool forceQuit = false);
    bool queryCacheExisted(const QString &hash, quint64 requestGeneration);
    void clearRequestIfCurrent(quint64 requestGeneration);
    bool stopRequested(quint64 requestGeneration) const;
    void run() override;

private:
    mutable QMutex    m_mutex;
    QString           m_curPath;
    QString           m_curHash;
    QString           m_pendingPath;
    QString           m_pendingHash;
    QVector<float>    m_listData;
    quint64           m_requestGeneration = 0;
    quint64           m_activeGeneration = 0;
    bool              m_hasPendingRequest = false;
    bool              m_stopFlag = false;
    bool              m_shuttingDown = false;
};
