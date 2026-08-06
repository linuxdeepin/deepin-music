// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mediametaworker.h"

#include "audioanalysis.h"
#include "util/log.h"

MediaMetaWorker::MediaMetaWorker(QObject *parent)
    : QObject(parent)
{
    qCDebug(dmMusic) << "MediaMetaWorker initialized";
}

void MediaMetaWorker::enqueueMetas(const QList<DMusic::MediaMeta> &metas)
{
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

    processQueue();
}

void MediaMetaWorker::processQueue()
{
    while (!m_queue.isEmpty()) {
        DMusic::MediaMeta meta = m_queue.dequeue();
        m_pendingHashes.remove(meta.hash);

        AudioAnalysis::parseMetaCoverAndLyrics(meta);
        m_doneHashes.insert(meta.hash);
        emit signalMetaAnalysisReady(meta);
    }
}
