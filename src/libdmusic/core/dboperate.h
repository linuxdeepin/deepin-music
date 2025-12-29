// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBOPERATE_H
#define DBOPERATE_H

#include "global.h"
#include <QSet>

class DBOperate : public QObject
{
    Q_OBJECT
public:
    DBOperate(QStringList supportedSuffixs, QObject *parent = nullptr);

public slots:
    void slotImportMetas(const QStringList &urls, const QSet<QString> &metaHashs, bool importPlay,
                         const QSet<QString> &playMetaHashs, const QSet<QString> &allMetaHashs,
                         const QString &playlistHash = "", const bool &playFalg = false);
    void slotClearImportingHash(const QString &hash);

signals:
    void signalAddOneMeta(QStringList playlistHashs, DMusic::MediaMeta meta);
    void signalImportFinished(QStringList playlistHashs, int failCount, int sucessCount, int existCount, QString mediaHash);

private:
    QStringList          m_supportedSuffixs;
    QSet<QString>        m_importingHashes;  // 记录已发起导入的文件 hash，防止短时间内重复解析
};

#endif //DBOPERATE_H
