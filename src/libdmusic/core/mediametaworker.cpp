// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mediametaworker.h"

#include "audioanalysis.h"
#include "util/log.h"

#include <QTimer>

MediaMetaWorker::MediaMetaWorker(QObject *parent)
    : QObject(parent)
{
    qCDebug(dmMusic) << "MediaMetaWorker initialized";
}

void MediaMetaWorker::requestStop()
{
    m_stopRequested.store(true, std::memory_order_release);
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

        m_queue.enqueue(meta);
        m_pendingHashes.insert(meta.hash);
    }

    scheduleNext();
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

    DMusic::MediaMeta meta = m_queue.dequeue();
    m_pendingHashes.remove(meta.hash);

    AudioAnalysis::parseMetaCoverAndLyrics(meta);
    m_doneHashes.insert(meta.hash);
    emit signalMetaAnalysisReady(meta);

    scheduleNext();
}
