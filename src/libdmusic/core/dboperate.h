// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBOPERATE_H
#define DBOPERATE_H

#include "global.h"

class DBOperate : public QObject
{
    Q_OBJECT
public:
    DBOperate(QStringList supportedSuffixs, QObject *parent = nullptr);

public slots:
    void slotImportMetas(const QStringList &urls, const QSet<QString> &metaHashs, bool importPlay,
                         const QSet<QString> &playMetaHashs, const QSet<QString> &allMetaHashs,
                         const QString &playlistHash = "", const bool &playFalg = false);

signals:
    void signalAddOneMeta(QStringList playlistHashs, DMusic::MediaMeta meta);
    void signalImportFinished(QStringList playlistHashs, int failCount, int sucessCount, int existCount, QString mediaHash);

private:
    QStringList          m_supportedSuffixs;
};

#endif //DBOPERATE_H
