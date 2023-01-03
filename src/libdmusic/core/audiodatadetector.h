// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QThread>
#include <QVector>
#include <QRandomGenerator>
#include <numeric>

class AudioDataDetector : public QThread
{
    Q_OBJECT
public:
    explicit AudioDataDetector(QObject *parent = Q_NULLPTR);
    ~AudioDataDetector();

public slots:
    void onBufferDetector(const QString &path, const QString &hash);
    void onClearBufferDetector();

signals:
    void audioBuffer(const QVector<float> &buffer, const QString &hash);
    void audioBufferFromThread(const QVector<float> &buffer, const QString &hash);

private:
    void resample(const QVector<float> &buffer, const QString &hash, bool forceQuit = false);
    bool queryCacheExisted(const QString &hash);
    void run() override;

private:
    QString           m_curPath;
    QString           m_curHash;
    QVector<float>    m_listData;
    bool              m_stopFlag = false;
};
