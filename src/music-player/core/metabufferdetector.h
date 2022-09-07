// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QThread>
#include <QVector>
#include <QRandomGenerator>
#include <numeric>

#include "libdmusic_global.h"

class MetaBufferDetectorPrivate;
class LIBDMUSICSHARED_EXPORT MetaBufferDetector : public QThread
{
    Q_OBJECT
public:
    explicit MetaBufferDetector(QObject *parent = Q_NULLPTR);
    ~MetaBufferDetector();
public slots:
    void onBufferDetector(const QString &path, const QString &hash);
    void onClearBufferDetector();

signals:
    void metaBuffer(const QVector<float> &buffer, const QString &hash);

private:
    void resample(const QVector<float> &buffer, const QString &hash, bool forceQuit = false);
    int queryCacheExisted(const QString &hash);
    void run() override;
private:
//    QScopedPointer<MetaBufferDetectorPrivate> d_ptr;
//    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MetaBufferDetector)
    QString           m_curPath;
    QString           m_curHash;
    QVector<float>    m_listData;

    bool              m_stopFlag = false;
};
