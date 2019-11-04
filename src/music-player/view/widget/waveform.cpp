/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *               2011 ~ 2018 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDebug>
#include <QEvent>
#include <QPaintEvent>
#include <QApplication>
#include <QPainter>
#include <QAudioBuffer>
#include <QAudioFormat>
#include <QWidget>

#include "waveform.h"
#include "waveformscale.h"
#include "core/util/fft.h"

const int Waveform::SAMPLE_DURATION = 30;
const int Waveform::WAVE_WIDTH = 2;
const int Waveform::WAVE_DURATION = 4;

Waveform::Waveform(Qt::Orientation orientation, QWidget *widget, QWidget *parent) : mainWindow(widget), DSlider(orientation, parent)
{
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setSizePolicy(sp);
    setFixedHeight(40);
    maxSampleNum = 16;
    slider()->hide();

    waveformScale = new WaveformScale(mainWindow);
    waveformScale->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    waveformScale->hide();
}

void Waveform::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.save();
    painter.setPen(Qt::NoPen);
    qreal devicePixelRatio = qApp->devicePixelRatio();
    if (devicePixelRatio > 1.0) {
        painter.setClipRect(QRect(rect().x(), rect().y(), rect().width() - 1, rect().height()));
    } else {
        painter.setClipRect(QRect(rect().x(), rect().y(), rect().width(), rect().height()));
    }

    int volume = 0;
    double curWidth = rect().width() * (value() * 1.0) / (maximum() - minimum());
    //draw left
    QColor fillColor(Qt::black);
    if (themeType == 2)
        fillColor = QColor("#FFFFFF");
    painter.save();
    if (devicePixelRatio > 1.0) {
        painter.setClipRect(QRect(rect().x(), rect().y(), curWidth - 1, rect().height()));
    } else {
        painter.setClipRect(QRect(rect().x(), rect().y(), curWidth, rect().height()));
    }
    for (int i = 0; i < sampleList.size(); i++) {
        volume = sampleList[i] * rect().height();
        if (volume == 0) {
            QPainterPath path;
            path.addRect(QRectF(rect().x() + i * WAVE_DURATION, rect().y() + (rect().height() - 1), WAVE_DURATION, 1));
            painter.fillPath(path, fillColor);
        } else {
            QRect sampleRect(rect().x() + i * WAVE_DURATION, rect().y() + (rect().height() - 1), WAVE_WIDTH, -qAbs(volume));
            painter.fillRect(sampleRect, fillColor);
        }
    }
    if (sampleList.size() < curWidth / WAVE_DURATION) {
        QPainterPath path;
        path.addRect(QRectF(rect().x() + sampleList.size() * WAVE_DURATION,
                            rect().y() + (rect().height() - 1),
                            curWidth - (sampleList.size() * WAVE_DURATION),
                            1));
        painter.fillPath(path, fillColor);
    }
    painter.restore();

    //draw right
    fillColor = QColor("#000000");
    if (themeType == 2)
        fillColor = QColor("#FFFFFF");
    fillColor.setAlphaF(0.5);
    painter.save();
    if (devicePixelRatio > 1.0) {
        painter.setClipRect(QRect(rect().x() + curWidth - 1, rect().y(), rect().width() - (curWidth - 1), rect().height()));
    } else {
        painter.setClipRect(QRect(rect().x() + curWidth, rect().y(), rect().width() - curWidth, rect().height()));
    }
    for (int i = 0; i < sampleList.size(); i++) {
        volume = sampleList[i] * rect().height();
        if (volume == 0) {
            QPainterPath path;
            path.addRect(QRectF(rect().x() + i * WAVE_DURATION, rect().y() + (rect().height() - 1), WAVE_DURATION, 1));
            painter.fillPath(path, fillColor);
        } else {
            QRect sampleRect(rect().x() + i * WAVE_DURATION, rect().y() + (rect().height() - 1), WAVE_WIDTH, -qAbs(volume));
            painter.fillRect(sampleRect, fillColor);
        }
    }
    if (sampleList.size() < rect().width() / WAVE_DURATION) {
        fillColor = Qt::darkGray;
        QPainterPath path;
        path.addRect(QRectF(rect().x() + sampleList.size() * WAVE_DURATION,
                            rect().y() + (rect().height() - 1),
                            rect().width() - (sampleList.size() * WAVE_DURATION),
                            1));
        painter.fillPath(path, fillColor);
    }
    painter.restore();

    painter.restore();
}

void Waveform::clearWave()
{
    sampleList.clear();
}

void Waveform::onAudioBufferProbed(const QAudioBuffer &buffer)
{
    for (auto value : getBufferLevels(buffer)) {
        reciveSampleList.push_front(value);
        break;
    }
    if (reciveSampleList.size() > maxSampleNum)
        reciveSampleList.pop_back();

    powerSpectrum();
    update();

//    if (width() > maxSampleNum)
//        maxSampleNum = width();
//    if (reciveSampleList.size() == maxSampleNum) {
//        sampleList = reciveSampleList;
//        reciveSampleList.clear();
//    }

//    updateScaleSize();
//    update();
}

// returns the audio level for each channel
QVector<qreal> Waveform::getBufferLevels(const QAudioBuffer &buffer)
{
    QVector<qreal> values;

    if (!buffer.format().isValid() || buffer.format().byteOrder() != QAudioFormat::LittleEndian)
        return values;

    if (buffer.format().codec() != "audio/pcm")
        return values;

    int channelCount = buffer.format().channelCount();
    values.fill(0, channelCount);
    qreal peak_value = Waveform::getPeakValue(buffer.format());
    if (qFuzzyCompare(peak_value, qreal(0)))
        return values;

    switch (buffer.format().sampleType()) {
    case QAudioFormat::Unknown:
    case QAudioFormat::UnSignedInt:
        if (buffer.format().sampleSize() == 32)
            values = Waveform::getBufferLevels(buffer.constData<quint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = Waveform::getBufferLevels(buffer.constData<quint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = Waveform::getBufferLevels(buffer.constData<quint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] = qAbs(values.at(i) - peak_value / 2) / (peak_value / 2);
        break;
    case QAudioFormat::Float:
        if (buffer.format().sampleSize() == 32) {
            values = Waveform::getBufferLevels(buffer.constData<float>(), buffer.frameCount(), channelCount);
            for (int i = 0; i < values.size(); ++i)
                values[i] /= peak_value;
        }
        break;
    case QAudioFormat::SignedInt:
        if (buffer.format().sampleSize() == 32)
            values = Waveform::getBufferLevels(buffer.constData<qint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = Waveform::getBufferLevels(buffer.constData<qint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = Waveform::getBufferLevels(buffer.constData<qint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] /= peak_value;
        break;
    }

    return values;
}

template <class T>
QVector<qreal> Waveform::getBufferLevels(const T *buffer, int frames, int channels)
{
    QVector<qreal> max_values;
    max_values.fill(0, channels);

    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < channels; ++j) {
            qreal value = qAbs(qreal(buffer[i * channels + j]));
            if (value > max_values.at(j))
                max_values.replace(j, value);
        }
    }

    return max_values;
}

// This function returns the maximum possible sample value for a given audio format
qreal Waveform::getPeakValue(const QAudioFormat &format)
{
    // Note: Only the most common sample formats are supported
    if (!format.isValid())
        return qreal(0);

    if (format.codec() != "audio/pcm")
        return qreal(0);

    switch (format.sampleType()) {
    case QAudioFormat::Unknown:
        break;
    case QAudioFormat::Float:
        if (format.sampleSize() != 32) // other sample formats are not supported
            return qreal(0);
        return qreal(1.00003);
    case QAudioFormat::SignedInt:
        if (format.sampleSize() == 32)
            return qreal(INT_MAX);
        if (format.sampleSize() == 16)
            return qreal(SHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(CHAR_MAX);
        break;
    case QAudioFormat::UnSignedInt:
        if (format.sampleSize() == 32)
            return qreal(UINT_MAX);
        if (format.sampleSize() == 16)
            return qreal(USHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(UCHAR_MAX);
        break;
    }

    return qreal(0);
}

void Waveform::mouseReleaseEvent(QMouseEvent *event)
{
    this->blockSignals(false);
    DSlider::mouseReleaseEvent(event);
    Q_EMIT valueAccpet(value());
    updateScaleSize();
}

void Waveform::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton
            || event->button() == Qt::MiddleButton
            || event->button() == Qt::RightButton) {
        if (orientation() == Qt::Vertical) {
            setValue(minimum() + ((maximum() - minimum()) * (height() - event->y())) / height()) ;
        } else {
            setValue(minimum() + ((maximum() - minimum()) * (event->x())) / (width()));
        }
    }
    updateScaleSize();
    this->blockSignals(true);
}

void Waveform::mouseMoveEvent(QMouseEvent *event)
{
    auto valueRange = this->maximum()  - this->minimum();
    auto viewRange = this->width();

    if (0 == viewRange) {
        return;
    }

    auto value = minimum() + ((maximum() - minimum()) * (event->x())) / (width());
    setValue(value);
    updateScaleSize();
}

void Waveform::onProgressChanged(qint64 value, qint64 duration)
{
    auto length = maximum() - minimum();
    Q_ASSERT(length != 0);

    auto progress = 0;
    if (0 != duration) {
        progress = static_cast<int>(length * value / duration);
    }

    if (signalsBlocked()) {
        return;
    }

    curValue = value;
    allDuration = duration;
    blockSignals(true);
    setValue(progress);
    blockSignals(false);
    update();
    updateScaleSize();
}

void Waveform::updateScaleSize()
{
    auto waveScaleWidth = waveformScale->width();
    double curWidth = rect().width() * (value() * 1.0) / (maximum() - minimum());
    curValue = allDuration * (value() * 1.0) / (maximum() - minimum());

    auto wavePos = mapToParent(QPoint(curWidth - waveScaleWidth / 2, 0));
    wavePos.ry() = -35;
    wavePos = ((QWidget *)parent())->mapToGlobal(wavePos);
    wavePos = mainWindow->mapFromGlobal(wavePos);

    waveformScale->move(wavePos.x(), wavePos.y());
    waveformScale->setValue(curValue);
    waveformScale->update();
}

void Waveform::setThemeType(int type)
{
    themeType = type;
}

void Waveform::enterEvent(QEvent *event)
{
    updateScaleSize();
    waveformScale->show();
    waveformScale->raise();

    DSlider::enterEvent(event);
}

void Waveform::leaveEvent(QEvent *event)
{
    waveformScale->hide();
    DSlider::leaveEvent(event);
}

bool Waveform::powerSpectrum()
{
    sampleList.clear();
    if (reciveSampleList.size() != maxSampleNum)
        return false;

    complex<float> *sample;

    sample = new complex<float>[maxSampleNum];
    for (int i = 0; i < maxSampleNum; i++)
        sample[i] = complex<float>(reciveSampleList[i]/* / 32768.0*/, 0);

    int log2N = log2(maxSampleNum - 1) + 1;
    int sign = -1;

    CFFT::process(sample, log2N, sign);

    QVector<float> curSampleListX, curSampleListY;
    for (int i = 0; i < maxSampleNum; i++) {
        curSampleListY.append(abs(sample[i]) / sqrt(2) / 2);
        if (curSampleListY[i] < 0 || curSampleListY[i] > 1)
            curSampleListY[i] = 0;
    }
    curSampleListY = curSampleListY.mid(curSampleListY.size() / 2 - 3, 7);
    int singleWidth = width() / (curSampleListY.size() - 1);
    for (int i = 0; i < curSampleListY.size(); i++) {
        curSampleListX.append(i * singleWidth);
    }

    QVector<float> endSampleListX, endSampleListY;
    spline(curSampleListX, curSampleListY, endSampleListX, sampleList, width() / WAVE_DURATION + 1);
    delete [] sample;
    return true;
}

void Waveform::spline(QVector<float> &x, QVector<float> &y, QVector<float> &vx, QVector<float> &vy, int pnt)
{
    vx.clear();
    vy.clear();

    QVector<float> tx = x;
    QVector<float> ty = y;

    for (int i = 1; i < x.size();) {
        if (fabs(x[i] - x[i - 1]) < 0.01) {
            x.erase(x.begin() + i);
            y.erase(y.begin() + i);
            continue;
        }

        i ++;
    }

    int N = x.size();
    if (N != y.size()) return;

    if (N == 1) {
        vx = x;
        vy = y;
        return;
    }

    if (N == 2) {
        x.insert(x.begin() + 1, (x[0] + x[1]) / 2);
        y.insert(y.begin() + 1, (y[0] + y[1]) / 2);
    }

    QVector<float> h;
    h.resize(N - 1);
    for (int i = 0; i < N - 1; i ++) h[i] = x[i + 1] - x[i];

    QVector<float> M;
    M.resize(N);
    M[0] = 0;
    M[N - 1] = 0;
    for (int i = 1; i < N - 1; i ++) M[i] = 6 * ((y[i + 1] - y[i]) / h[i] - (y[i] - y[ i - 1]) / h[i - 1]);

    QVector<QVector<float> > A;
    A.resize(N);
    for (int i = 0; i < N; i ++) A[i].resize(3);

    A[0][0] = 1;
    A[N - 1][2] = 1;

    for (int i = 1; i < N - 1; i ++) {
        A[i][0] = h[i - 1];
        A[i][1] = 2 * (h[i - 1] + h[i]);
        A[i][2] = h[i];
    }

    QVector<float> C;
    C.resize(N);
    C[0] = A[0][1] / A[0][0];
    for (int i = 1; i < N; i ++) C[i] = A[i][2] / (A[i][1] - C[i - 1] * A[i][0]);

    QVector<float> D;
    D.resize(N);
    D[0] = M[0] / A[0][0];
    for (int i = 1; i < N; i ++) D[i] = (M[i] - D[i - 1] * A[i][0]) / (A[i][1] - C[i - 1] * A[i][0]);

    QVector<float> m;
    m.resize(N);
    m[N - 1] = 0;

    for (int i = N - 2; i >= 0; i --) m[i] = D[i] - C[i] * m[i + 1];

    QVector<float> a, b, c, d;
    a.resize(N - 1);
    b.resize(N - 1);
    c.resize(N - 1);
    d.resize(N - 1);
    for (int i = 0; i < N - 1; i ++) {
        a[i] = y[i];
        b[i] = (y[ i + 1] - y[i]) / h[i] - h[i] * m[i] / 2 - h[i] * (m[i + 1] - m[i]) / 6;
        c[i] = m[i] / 2;
        d[i] = (m[i + 1] - m[i]) / (6 * h[i]);
    }

    vx.resize(pnt);
    vy.resize(pnt);

    int index = 0;
    for (int i = 0; i < pnt; i ++) {
        vx[i] = i * (x[N - 1] - x[0]) / (pnt - 1) + x[0];

        while (vx[i] > x[index + 1]) index ++;
        float fx = vx[i] - x[index];

        vy[i] = a[index] + b[index] * fx + c[index] * fx * fx + d[index] * fx * fx * fx;
    }
    x = tx;
    y = ty;
}
