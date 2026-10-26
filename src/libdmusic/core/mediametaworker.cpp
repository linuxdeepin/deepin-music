// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mediametaworker.h"

#include "audioanalysis.h"
#include "util/log.h"

#include <QTimer>
#include <QMutexLocker>

MediaMetaWorker::MediaMetaWorker(QObject *parent)
    : QObject(parent)
{
    qCDebug(dmMusic) << "MediaMetaWorker initialized";
}

void MediaMetaWorker::requestStop()
{
    m_stopRequested.store(true, std::memory_order_release);
}

void MediaMetaWorker::invalidateMetaHash(const QString &hash)
{
    QMutexLocker locker(&m_generationMutex);
    m_taskGenerations.remove(hash);
}

bool MediaMetaWorker::isCurrentGeneration(const QString &hash, quint64 generation) const
{
    QMutexLocker locker(&m_generationMutex);
    return m_taskGenerations.value(hash, 0) == generation;
}

void MediaMetaWorker::enqueueMetas(const QList<DMusic::MediaMeta> &metas)
{
    if (m_stopRequested.load(std::memory_order_acquire)) {
        return;
    }

    for (const DMusic::MediaMeta &meta : metas) {
        if (meta.hash.isEmpty() || meta.localPath.isEmpty()) {
            continue;
        }
        if (m_doneHashes.contains(meta.hash) || m_pendingHashes.contains(meta.hash)) {
            qCDebug(dmMusic) << "Metadata task already queued or finished:" << meta.hash;
            continue;
        }

        const quint64 taskId = ++m_nextTaskId;
        quint64 generation;
        {
            QMutexLocker locker(&m_generationMutex);
            generation = ++m_nextGeneration;
            m_taskGenerations.insert(meta.hash, generation);
        }
        m_queue.enqueue({meta, taskId, generation});
        m_taskIds.insert(meta.hash, taskId);
        m_pendingHashes.insert(meta.hash);
    }

    scheduleNext();
}

// A deleted track may be reimported with the same hash, so invalidate stale dedup state.
void MediaMetaWorker::clearMetaHash(const QString &hash)
{
    m_doneHashes.remove(hash);
    m_pendingHashes.remove(hash);
    m_taskIds.remove(hash);
}

void MediaMetaWorker::scheduleNext()
{
    if (m_stopRequested.load(std::memory_order_acquire)
        || m_processScheduled || m_queue.isEmpty()) {
        return;
    }

    m_processScheduled = true;
    QTimer::singleShot(0, this, &MediaMetaWorker::processNext);
}

void MediaMetaWorker::processNext()
{
    m_processScheduled = false;
    if (m_stopRequested.load(std::memory_order_acquire) || m_queue.isEmpty()) {
        return;
    }

    const MetaTask task = m_queue.dequeue();
    if (m_taskIds.value(task.meta.hash) != task.id) {
        scheduleNext();
        return;
    }

    m_taskIds.remove(task.meta.hash);
    m_pendingHashes.remove(task.meta.hash);
    DMusic::MediaMeta meta = task.meta;
    AudioAnalysis::parseMetaCoverAndLyrics(meta);
    if (!isCurrentGeneration(meta.hash, task.generation)) {
        scheduleNext();
        return;
    }
    m_doneHashes.insert(meta.hash);
    emit signalMetaAnalysisReady(meta, task.generation);

    scheduleNext();
}
