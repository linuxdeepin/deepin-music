// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dboperate.h"

#include <QFileInfo>
#include <QDirIterator>
#include <QDateTime>
#include <QStandardPaths>
#include <QDebug>
#include <QtConcurrent>

#include "audioanalysis.h"
#include "utils.h"
#include "util/log.h"

using namespace DMusic;

DBOperate::DBOperate(QStringList supportedSuffixs, QObject *parent)
    : QObject(parent)
{
    qCDebug(dmMusic) << "Initializing DBOperate with supported suffixes:" << supportedSuffixs;
    for (QString str : supportedSuffixs) {
        m_supportedSuffixs.append("*." + str);
        qCDebug(dmMusic) << "Added supported suffix:" << "*." + str;
    }
    qCDebug(dmMusic) << "DBOperate initialized with" << m_supportedSuffixs.size() << "supported suffixes";
}

void DBOperate::slotImportMetas(const QStringList &urls, const QSet<QString> &metaHashs, bool importPlay,
                                const QSet<QString> &playMetaHashs, const QSet<QString> &allMetaHashs,
                                const QString &playlistHash, const bool &playFalg)
{
    qCDebug(dmMusic) << "Starting meta import with" << urls.size() << "URLs, playlist:" << playlistHash;

    // 清理已确认成功添加的 hash（这些 hash 已在 m_allMetas 中，不再需要跟踪）
    for (const QString &hash : allMetaHashs) {
        m_importingHashes.remove(hash);
    }

    QString mediaHash;
    QStringList filePaths;
    QStringList allUrls = urls;
    if (allUrls.isEmpty()) {
        allUrls.append(DmGlobal::musicPath());
        qCDebug(dmMusic) << "No URLs provided, using default music path:" << DmGlobal::musicPath();
    }

    // Phase 1: Serial file scan (fast, I/O-bound on directory listing)
    for (auto &curUrl : allUrls) {
        if (curUrl.isEmpty()) {
            continue;
        }
        QUrl url(curUrl);
        QString filepath = url.toLocalFile().isEmpty() ? curUrl : url.toLocalFile();
        QFileInfo fileInfo(filepath);
        if (fileInfo.isDir()) {
            qCDebug(dmMusic) << "Scanning directory:" << filepath;
            QDirIterator it(filepath, m_supportedSuffixs,
                            QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                filePaths.append(it.next());
            }
            qCDebug(dmMusic) << "Finished scanning directory, found" << filePaths.size() << "files";
        } else {
            filePaths.append(filepath);
            qCDebug(dmMusic) << "Added single file:" << filepath;
        }
    }
    qCDebug(dmMusic) << "Found" << filePaths.size() << "files to process";

    // Phase 2: Classify files — determine which need parsing vs already known
    struct ParseTask {
        QString filePath;
        QString hash;
    };
    enum FileClass { Skip, ExistsPlaylist, ExistsAll, NeedsParse };
    struct FileEntry {
        QString hash;
        FileClass fileClass;
    };

    QList<FileEntry> entries;
    QList<ParseTask> parseTasks;

    for (auto &filePath : filePaths) {
        QFileInfo fileinfo(filePath);
        while (fileinfo.isSymLink()) {
            fileinfo.setFile(fileinfo.symLinkTarget());
        }
        auto hash = Utils::filePathHash(fileinfo.absoluteFilePath());

        if (m_importingHashes.contains(hash)) {
            qCDebug(dmMusic) << "File already being imported by another task, skipping:" << filePath;
            continue;
        }

        FileEntry entry;
        entry.hash = hash;

        if (!playlistHash.isEmpty()) {
            if (metaHashs.contains(hash)) {
                entry.fileClass = ExistsPlaylist;
            } else if (!allMetaHashs.contains(hash)) {
                entry.fileClass = NeedsParse;
                m_importingHashes.insert(hash);
                parseTasks.append({filePath, hash});
            } else {
                entry.fileClass = ExistsAll;
            }
        } else {
            if (!allMetaHashs.contains(hash)) {
                entry.fileClass = NeedsParse;
                m_importingHashes.insert(hash);
                parseTasks.append({filePath, hash});
            } else {
                entry.fileClass = ExistsAll;
            }
        }
        entries.append(entry);
    }

    // Phase 3: Parallel metadata parsing (CPU-bound, ~384ms/file → multi-threaded)
    QHash<QString, DMusic::MediaMeta> parsedMap;
    if (!parseTasks.isEmpty()) {
        qCDebug(dmMusic) << "Parsing" << parseTasks.size() << "files in parallel";
        auto parseFunc = [](const ParseTask &task) -> DMusic::MediaMeta {
            return AudioAnalysis::creatMediaMeta(task.filePath, task.hash);
        };
        QList<DMusic::MediaMeta> parsedResults = QtConcurrent::blockingMapped(parseTasks, parseFunc);
        for (const auto &meta : parsedResults) {
            parsedMap.insert(meta.hash, meta);
        }
        qCDebug(dmMusic) << "Parallel parsing complete," << parsedMap.size() << "results";
    }

    // Phase 4: Serial post-processing — build results and emit signals
    int importedCount = 0, importedFailCount = 0, existCount = 0;
    QSet<QString> allHashs;
    QList<DMusic::MediaMeta> pendingMetas;

    for (const auto &entry : entries) {
        DMusic::MediaMeta mediaMeta;
        mediaMeta.hash = entry.hash;
        QSet<QString> curHashs;

        switch (entry.fileClass) {
        case ExistsPlaylist:
            existCount++;
            break;
        case ExistsAll:
            if (!playlistHash.isEmpty()) {
                curHashs << playlistHash;
                allHashs << playlistHash;
            } else {
                existCount++;
            }
            break;
        case NeedsParse: {
            auto it = parsedMap.find(entry.hash);
            if (it != parsedMap.end()) {
                mediaMeta = it.value();
            }
            if (mediaMeta.length > 0) {
                mediaMeta.hasimage = false;
                pendingMetas.append(mediaMeta);
                if (!playlistHash.isEmpty()) {
                    curHashs << "all" << playlistHash;
                    allHashs << "all" << playlistHash;
                } else {
                    curHashs << "all";
                    allHashs << "all";
                }
                qCDebug(dmMusic) << "Added new meta:" << mediaMeta.title;
            } else {
                importedFailCount++;
                qCWarning(dmMusic) << "Failed to import file with hash:" << entry.hash;
            }
            break;
        }
        default:
            break;
        }

        if (mediaMeta.length > 0 && importPlay && !playMetaHashs.contains(mediaMeta.hash)) {
            curHashs << "play";
            allHashs << "play";
        }
        if (!curHashs.isEmpty())
            emit signalAddOneMeta(curHashs.values(), mediaMeta);
        if (mediaHash.isEmpty() && playFalg && mediaMeta.length > 0)
            mediaHash = mediaMeta.hash;

        importedCount++;
    }

    emit signalImportFinished(allHashs.values(), importedFailCount, importedCount - importedFailCount - existCount, existCount, mediaHash);

    if (!pendingMetas.isEmpty()) {
        emit signalEnqueueMetaTasks(pendingMetas);
    }

    emit signalMetaBatchFinished();
}

void DBOperate::slotClearImportingHash(const QString &hash)
{
    m_importingHashes.remove(hash);
}
