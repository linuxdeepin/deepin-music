// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEDIAMETAWORKER_H
#define MEDIAMETAWORKER_H

#include "global.h"

#include <QObject>
#include <QQueue>
#include <QSet>

#include <atomic>

class MediaMetaWorker : public QObject
{
    Q_OBJECT
public:
    explicit MediaMetaWorker(QObject *parent = nullptr);

    // Can be called from the owner thread while the worker is parsing. The
    // current file is allowed to finish, but no queued file is started.
    void requestStop();

public slots:
    void enqueueMetas(const QList<DMusic::MediaMeta> &metas);

signals:
    void signalMetaAnalysisReady(DMusic::MediaMeta meta);

private slots:
    void processNext();

private:
    void scheduleNext();

private:
    QQueue<DMusic::MediaMeta> m_queue;
    QSet<QString> m_pendingHashes;
    QSet<QString> m_doneHashes;
    bool m_processScheduled = false;
    std::atomic_bool m_stopRequested{false};
};

#endif // MEDIAMETAWORKER_H
