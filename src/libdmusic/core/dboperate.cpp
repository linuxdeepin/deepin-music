// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dboperate.h"

#include <QFileInfo>
#include <QDirIterator>
#include <QDateTime>
#include <QStandardPaths>
#include <QDebug>

#include "audioanalysis.h"
#include "utils.h"

using namespace DMusic;

DBOperate::DBOperate(QStringList supportedSuffixs, QObject *parent)
    : QObject(parent)
{
    for (QString str : supportedSuffixs) {
        m_supportedSuffixs.append("*." + str);
    }
}

void DBOperate::slotImportMetas(const QStringList &urls, const QSet<QString> &metaHashs, bool importPlay,
                                const QSet<QString> &playMetaHashs, const QSet<QString> &allMetaHashs,
                                const QString &playlistHash, const bool &playFalg)
{
    QString mediaHash;
    // 统计总共需要加载的数量
    QStringList filePaths;
    QStringList allUrls = urls;
    // 添加默认音乐目录
    if (allUrls.isEmpty())
        allUrls.append(DmGlobal::musicPath());
    for (auto &curUrl : allUrls) {
        if (curUrl.isEmpty()) {
            continue;
        }
        QUrl url(curUrl);
        QString filepath = url.toLocalFile().isEmpty() ? curUrl : url.toLocalFile();
        QFileInfo fileInfo(filepath);
        if (fileInfo.isDir()) {
            QDirIterator it(filepath, m_supportedSuffixs,
                            QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                filePaths.append(it.next());
            }
        } else {
            filePaths.append(filepath);
        }
    }
    qDebug() << __FUNCTION__ << "allCount = " << filePaths.size();

    int importedCount = 0, importedFailCount = 0, existCount = 0;
    QSet<QString> allHashs;
    for (auto &filePath : filePaths) {
        QFileInfo fileinfo(filePath);
        while (fileinfo.isSymLink()) {  //to find final target
            fileinfo.setFile(fileinfo.symLinkTarget());
        }
        auto hash = Utils::filePathHash(fileinfo.absoluteFilePath());
        DMusic::MediaMeta mediaMeta;
        mediaMeta.hash = hash;
        QSet<QString> curHashs;
        if (!playlistHash.isEmpty()) {
            if (metaHashs.contains(mediaMeta.hash)) {
                existCount++;
            } else if (!allMetaHashs.contains(mediaMeta.hash)) {
                mediaMeta = AudioAnalysis::creatMediaMeta(filePath);
                if (mediaMeta.length > 0) {
                    AudioAnalysis::parseMetaCover(mediaMeta);
                    AudioAnalysis::parseMetaLyrics(mediaMeta);
                    curHashs << "all" << playlistHash;
                    allHashs << "all" << playlistHash;
                } else {
                    importedFailCount++;
                }
            } else {
                curHashs << playlistHash;
                allHashs << playlistHash;
            }
        } else {
            if (!allMetaHashs.contains(mediaMeta.hash)) {
                mediaMeta = AudioAnalysis::creatMediaMeta(filePath);
                if (mediaMeta.length > 0) {
                    AudioAnalysis::parseMetaCover(mediaMeta);
                    AudioAnalysis::parseMetaLyrics(mediaMeta);
                    curHashs << "all";
                    allHashs << "all";
                } else {
                    importedFailCount++;
                }
            } else {
                existCount++;
            }
        }
        // 自动添加到播放列表
        if (mediaMeta.length > 0 && importPlay && !playMetaHashs.contains(mediaMeta.hash)) {
            curHashs << "play";
            allHashs << "play";
        }
        if (!curHashs.isEmpty())
            emit signalAddOneMeta(curHashs.values(), mediaMeta);
        // 自动播放
        if (mediaHash.isEmpty() && playFalg && mediaMeta.length > 0)
            mediaHash = mediaMeta.hash;

        importedCount++;
    }

    emit signalImportFinished(allHashs.values(), importedFailCount, importedCount - importedFailCount - existCount, existCount, mediaHash);
}
