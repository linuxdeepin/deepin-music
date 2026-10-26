// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEDIAMETAWORKER_H
#define MEDIAMETAWORKER_H

#include "global.h"

#include <QObject>
#include <QHash>
#include <QMutex>
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
    // These methods only access generation state and are safe from the owner thread.
    void invalidateMetaHash(const QString &hash);
    bool isCurrentGeneration(const QString &hash, quint64 generation) const;

public slots:
    void enqueueMetas(const QList<DMusic::MediaMeta> &metas);
    void clearMetaHash(const QString &hash);

signals:
    void signalMetaAnalysisReady(DMusic::MediaMeta meta, quint64 generation);

private slots:
    void processNext();

private:
    void scheduleNext();

private:
    struct MetaTask {
        DMusic::MediaMeta meta;
        quint64 id = 0;
        quint64 generation = 0;
    };

    QQueue<MetaTask> m_queue;
    QSet<QString> m_pendingHashes;
    QSet<QString> m_doneHashes;
    QHash<QString, quint64> m_taskIds;
    quint64 m_nextTaskId = 0;
    mutable QMutex m_generationMutex;
    QHash<QString, quint64> m_taskGenerations;
    quint64 m_nextGeneration = 0;
    bool m_processScheduled = false;
    std::atomic_bool m_stopRequested{false};
};

#endif // MEDIAMETAWORKER_H
