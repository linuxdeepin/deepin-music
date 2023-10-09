// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDebug>
#include <QEvent>
#include <QPaintEvent>
#include <QApplication>
#include <QPainter>
#include <QAudioBuffer>
#include <QAudioFormat>
#include <QWidget>
#include <QPainterPath>

#include "waveform.h"
#include "waveformscale.h"
#include "player.h"
#include "math.h"

const int Waveform::SAMPLE_DURATION = 30;
const int Waveform::WAVE_WIDTH = 2;
const int Waveform::WAVE_DURATION = 4;

Waveform::Waveform(Qt::Orientation orientation, QWidget *widget, QWidget *parent) : DSlider(orientation, parent), mainWindow(widget)
{
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setSizePolicy(sp);
    setFixedHeight(40);
    maxSampleNum = 16;
    slider()->hide();

    waveformScale = new WaveformScale(mainWindow);
    waveformScale->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    waveformScale->hide();

#ifdef DTKWIDGET_CLASS_DSizeMode
   slotSizeModeChanged(DGuiApplicationHelper::instance()->sizeMode());
#endif

    connect(Player::getInstance(), &Player::positionChanged, this, &Waveform::onProgressChanged);
    connect(Player::getInstance(), &Player::signalMediaStop, this, &Waveform::clearBufferAudio);
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(),&DGuiApplicationHelper::sizeModeChanged,this, &Waveform::slotSizeModeChanged);
#endif
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
    if (devicePixelRatio > 1.0) {
        painter.setClipRect(QRect(rect().x(), rect().y(), static_cast<int>(curWidth - 1), rect().height()));
    } else {
        painter.setClipRect(QRect(rect().x(), rect().y(), static_cast<int>(curWidth), rect().height()));
    }
    for (int i = 0; i < sampleList.size(); i++) {
        volume = static_cast<int>(sampleList[i] * rect().height());
        if (volume == 0) {
            volume = 1;
        }
        QRect sampleRect(rect().x() + i * WAVE_DURATION, rect().y() + (rect().height() - 1), WAVE_WIDTH, -qAbs(volume));
        painter.fillRect(sampleRect, fillColor);
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
    fillColor.setAlphaF(0.2);
    painter.save();
    if (devicePixelRatio > 1.0) {
        painter.setClipRect(QRect(rect().x() + static_cast<int>(curWidth - 1), rect().y(),
                                  rect().width() - static_cast<int>(curWidth - 1), rect().height()));
    } else {
        painter.setClipRect(QRect(rect().x() + static_cast<int>(curWidth), rect().y(),
                                  rect().width() - static_cast<int>(curWidth), rect().height()));
    }
    for (int i = 0; i < sampleList.size(); i++) {
        volume = static_cast<int>(sampleList[i] * rect().height());
        if (volume == 0) {
            volume = 1;
        }
        QRect sampleRect(rect().x() + i * WAVE_DURATION, rect().y() + (rect().height() - 1), WAVE_WIDTH, -qAbs(volume));
        painter.fillRect(sampleRect, fillColor);
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
}

void Waveform::onAudioBuffer(const QVector<float> &allData, const QString &hash)
{
    metaHash = hash;
    reciveSampleList = allData; //比例缩放已在MetaBufferDetector中处理
    spectrumFlag = false;
    updateAudioBuffer();
    this->update();
}

void Waveform::mouseReleaseEvent(QMouseEvent *event)
{
    this->blockSignals(false);
    DSlider::mouseReleaseEvent(event);

    updatePlayerPos(value());
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
    auto viewRange = this->width();

    if (0 == viewRange) {
        return;
    }

    auto value = minimum() + ((maximum() - minimum()) * (event->x())) / (width());
    setValue(value);
    updateScaleSize();
}

void Waveform::onProgressChanged(qint64 value, qint64 duration, qint64 coefficient)
{
    if (IsShowwaveformScale)
        waveformScale->show();
    auto length = maximum() - minimum();
    Q_ASSERT(length != 0);

    /*------curCoefficient-------*/
    curCoefficient = coefficient;
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
    curValue = static_cast<qint64>(allDuration * (value() * 1.0) / (maximum() - minimum()));

    auto wavePos = mapToParent(QPoint(static_cast<int>(curWidth - waveScaleWidth / 2), 0));
    wavePos.ry() = -35;
    wavePos = (static_cast<QWidget *>(parent())->mapToGlobal(wavePos));
    wavePos = mainWindow->mapFromGlobal(wavePos);

    waveformScale->move(wavePos.x(), wavePos.y());
    waveformScale->setValue(curValue * curCoefficient);

    waveformScale->update();
}

void Waveform::setThemeType(int type)
{
    themeType = type;
}

void Waveform::clearBufferAudio(const QString &hash)
{
    if (hash.isEmpty() || hash != metaHash) {
        reciveSampleList.clear();
        sampleList.clear();
        metaHash.clear();
    }
}

#ifdef DTKWIDGET_CLASS_DSizeMode
void Waveform::slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode)
{
    if (sizeMode == DGuiApplicationHelper::SizeMode::CompactMode) {
        setFixedHeight(29);
    } else {
        setFixedHeight(40);
    }
}
#endif

void Waveform::enterEvent(QEvent *event)
{
    IsShowwaveformScale = true;
    updateScaleSize();
    waveformScale->show();
    waveformScale->raise();

    DSlider::enterEvent(event);
}

void Waveform::leaveEvent(QEvent *event)
{
    IsShowwaveformScale = false;
    waveformScale->hide();
    // 隐藏刻度后可以显示进度
    if (signalsBlocked()) {
        blockSignals(false);
    }
    DSlider::leaveEvent(event);
}

void Waveform::resizeEvent(QResizeEvent *event)
{
    DSlider::resizeEvent(event);
    if (!spectrumFlag)
        updateAudioBuffer();
}

void Waveform::updateAudioBuffer()
{
    if (reciveSampleList.isEmpty()) {
        sampleList.clear();
        return;
    }
    QVector<float> curSampleListX;
    float singleWidth = width() / float(reciveSampleList.size() - 1);
    for (int i = 0; i < reciveSampleList.size(); i++) {
        curSampleListX.append(i * singleWidth);
    }
    QVector<float> endSampleListX, endSampleListY;
    spline(curSampleListX, reciveSampleList, endSampleListX, sampleList, width() / WAVE_DURATION + 1);
    update();
}

void Waveform::spline(QVector<float> &x, QVector<float> &y, QVector<float> &vx, QVector<float> &vy, int pnt)
{
    vx.clear();
    vy.clear();

    QVector<float> tx = x;
    QVector<float> ty = y;

    for (int i = 1; i < x.size();) {
        if (fabs(x[i] - x[i - 1]) < 0.01f) {
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
    for (int i = 0; i < (pnt - 1); i ++) {
        vx[i] = i * (x[N - 1] - x[0]) / (pnt - 1) + x[0];

        while (vx[i] > x[index + 1]) {
            index ++;
        }
        float fx = vx[i] - x[index];

        vy[i] = a[index] + b[index] * fx + c[index] * fx * fx + d[index] * fx * fx * fx;
    }
    x = tx;
    y = ty;
}

void Waveform::updatePlayerPos(int value)
{
    int range = this->maximum() - this->minimum();
    Q_ASSERT(range != 0);
    if (value <= range) {
        long long position = value * Player::getInstance()->duration() / range;
        if (Player::getInstance()->position() == -1) {
            auto activeMeta = Player::getInstance()->getActiveMeta();
            position = value * activeMeta.length / range;
        }
        Player::getInstance()->setPosition(position);
    }
}
