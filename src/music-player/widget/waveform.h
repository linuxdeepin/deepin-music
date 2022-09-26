// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <DSlider>

DWIDGET_USE_NAMESPACE

class QAudioBuffer;
class QAudioFormat;
class WaveformScale;

class Waveform : public DSlider
{
    Q_OBJECT

    static const int SAMPLE_DURATION;
    static const int WAVE_WIDTH;
    static const int WAVE_DURATION;

public:
    Waveform(Qt::Orientation orientation, QWidget *widget, QWidget *parent = nullptr);

//    static qreal getPeakValue(const QAudioFormat &format);
//    static QVector<qreal> getBufferLevels(const QAudioBuffer &buffer);

//    template <class T>
//    static QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels);

    //void clearWave();

    void updateScaleSize();
    void setThemeType(int type);
//    void hidewaveformScale();

public slots:
//    void onAudioBufferProbed(const QAudioBuffer &buffer);
    void onProgressChanged(qint64 value, qint64 duration, qint64 coefficient);
    void onAudioBuffer(const QVector<float> &allData, const QString &hash);
    void clearBufferAudio(const QString &hash = "");
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    void updateAudioBuffer();
//    bool powerSpectrum();
    void spline(QVector<float> &x, QVector<float> &y, QVector<float> &vx, QVector<float> &vy, int pnt);
    void updatePlayerPos(int value);
    //void isPlayNextMeta(int value);

private:
    QWidget      *mainWindow;
    QVector<float> sampleList;
    QVector<float> reciveSampleList;
    int          maxSampleNum;
    qint64       curValue = 0;
    qint64       allDuration = 1;
    qint64       curCoefficient = 1;
    int          themeType = 1;
    WaveformScale *waveformScale;
    bool         spectrumFlag = true;
    QString      metaHash;
    bool         IsShowwaveformScale = false;
};

#endif
