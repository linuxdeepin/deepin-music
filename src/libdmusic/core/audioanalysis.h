// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUDIOANALYSIS_H
#define AUDIOANALYSIS_H

#include <QObject>

#include "global.h"

class AudioAnalysisPrivate;
class AudioAnalysis : public QObject
{
    Q_OBJECT
public:
    AudioAnalysis(QObject *parent = nullptr);

    void parseAudioBuffer(const DMusic::MediaMeta &meta);

    static DMusic::MediaMeta creatMediaMeta(const QString &path);
    static void convertMetaCodec(DMusic::MediaMeta &meta, const QString &codecName);
    static bool parseMetaFromLocalFile(DMusic::MediaMeta &meta);
    static QStringList detectEncodings(const DMusic::MediaMeta &meta);

    static void parseMetaCover(DMusic::MediaMeta &meta);
    static QImage getMetaCoverImage(DMusic::MediaMeta meta);

    static void parseMetaLyrics(DMusic::MediaMeta &meta);

public slots:
    void startRecorder();
    void suspendRecorder();
    void stopRecorder();

signals:
    void audioSpectrumData(QVector<int> data);
    void audioBuffer(const QVector<float> &buffer, const QString &hash);

private:
    void parseData();
    static bool parseFileTagCodec(DMusic::MediaMeta &meta);

private:
    AudioAnalysisPrivate *m_data;
};

#endif //DATAMANAGER_H
