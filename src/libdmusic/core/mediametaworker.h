// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEDIAMETAWORKER_H
#define MEDIAMETAWORKER_H

#include "global.h"

#include <QObject>
#include <QQueue>
#include <QSet>

class MediaMetaWorker : public QObject
{
    Q_OBJECT
public:
    explicit MediaMetaWorker(QObject *parent = nullptr);

public slots:
    void enqueueMetas(const QList<DMusic::MediaMeta> &metas);

signals:
    void signalMetaAnalysisReady(DMusic::MediaMeta meta);

private:
    void processQueue();

private:
    QQueue<DMusic::MediaMeta> m_queue;
    QSet<QString> m_pendingHashes;
    QSet<QString> m_doneHashes;
};

#endif // MEDIAMETAWORKER_H
