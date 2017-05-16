#include "waterprogress.h"

#include <QPainter>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QtMath>
#include <QWidget>
#include <QDebug>

class WaterProgressInterPrivate
{
public:
    WaterProgressInterPrivate(WaterProgressInter *parent) : q_ptr(parent) {}

    void initUI();
    void setProgress(int p);
    void updateProcess();
    void paint(QPainter &p);

    int m_Progresss;
    int m_ForntXOffset1;
    int m_ForntXOffset2;
    int m_BackXOffset1;
    int m_BackXOffset2;
    int m_YOffset;
    int m_FrontWidth;
    int m_BackWidth;

    double m_Pop7YOffset;
    double m_Pop7XOffset;

    double m_Pop8YOffset;
    double m_Pop8XOffset;

    double m_Pop11YOffset;
    double m_Pop11XOffset;

    QImage m_shadow;
    QImage *m_BackImage;
    QImage *m_FrontImage;
    QTimer *m_timer;

    WaterProgressInter *q_ptr;
    Q_DECLARE_PUBLIC(WaterProgressInter)
};

WaterProgressInter::WaterProgressInter(QWidget *parent) :
    QWidget(parent), d_ptr(new WaterProgressInterPrivate(this))
{
    Q_D(WaterProgressInter);
    d->initUI();
}

WaterProgressInter::~WaterProgressInter()
{

}


void WaterProgressInter::setProgress(int p)
{
    Q_D(WaterProgressInter);
    d->setProgress(p);
}

void WaterProgressInter::start()
{
    Q_D(WaterProgressInter);
    d->m_timer->start(66);
}

void WaterProgressInter::stop()
{
    Q_D(WaterProgressInter);
    d->m_timer->stop();
}

void WaterProgressInter::paintEvent(QPaintEvent *)
{
    Q_D(WaterProgressInter);

    QPainter p(this);
    p.setRenderHint(QPainter::HighQualityAntialiasing);

    d->paint(p);
}

void WaterProgressInterPrivate::updateProcess()
{
    Q_Q(WaterProgressInter);

    int YMax = q->height();
    auto wmax = YMax;
    auto wmin = 12;

    double speed = 1.0 * m_Progresss / 100;

    m_ForntXOffset1 += 2;
    m_ForntXOffset2 += 2;
    if (m_ForntXOffset1 >= wmax) {
        m_ForntXOffset1 = -m_FrontWidth;
    }
    if (m_ForntXOffset2 >= wmax) {
        m_ForntXOffset2 = -m_FrontWidth;
    }

    m_BackXOffset1 -= 3;
    m_BackXOffset2 -= 3;
    if (m_BackXOffset1 < -m_BackWidth) {
        m_BackXOffset1 = wmax;
    }
    if (m_BackXOffset2 < -m_BackWidth) {
        m_BackXOffset2 = wmax;
    }

    if (m_YOffset < -wmin) {
        m_YOffset = 88;
    }

    m_Pop7YOffset -= 0.8 * speed;
    if (m_Pop7YOffset < m_YOffset + 10) {
        m_Pop7YOffset = 100;
    }
    m_Pop7XOffset = 35 + 14 + qSin((100 - m_Pop7YOffset) * 2 * 3.14 / 40) * 8;

    m_Pop8YOffset -= 1.2 * speed;
    if (m_Pop8YOffset < m_YOffset + 10) {
        m_Pop8YOffset = 100;
    }
    m_Pop8XOffset = 45 + 14 + qCos((100 - m_Pop8YOffset) * 2 * 3.14 / 40) * 9
                    * (100 - m_Pop8XOffset) / 60;

    m_Pop11YOffset -= 1.6 * speed;
    if (m_Pop11YOffset < m_YOffset + 10) {
        m_Pop11YOffset = 100;
    }
    m_Pop11XOffset = 55 + 14 + qSin((100 - m_Pop11YOffset) * 2 * 3.14 / 50)
                     * 11 * (100 - m_Pop11YOffset) / 60;

    q->update();
}

void WaterProgressInterPrivate::initUI()
{
    Q_Q(WaterProgressInter);

    q->setFixedSize(128, 128);
    q->setMask(QRegion(0, 0, q->width(), q->height(), QRegion::Ellipse));
    m_timer = new QTimer(q);
    q->connect(m_timer, &QTimer::timeout, q, [ = ]() {
        updateProcess();
    });

    m_FrontImage = new QImage(":/common/image/water_front.png");
    m_BackImage = new QImage(":/common/image/water_back.png");
    m_FrontWidth = m_FrontImage->width() - 1;
    m_BackWidth = m_BackImage->width() - 1;
    m_ForntXOffset1 = 0;
    m_ForntXOffset2 = -m_FrontWidth;
    m_BackXOffset1 = m_BackWidth / 2;
    m_BackXOffset2 = -m_BackWidth / 2;

    m_Progresss = 0;

    q->setProgress(m_Progresss);

    m_Pop7YOffset = 100;
    m_Pop8YOffset = 100;
    m_Pop11YOffset = 100;
}

void WaterProgressInterPrivate::setProgress(int p)
{
    Q_Q(WaterProgressInter);
    int YMax = q->height();
    int zeroY = YMax + 4;
    int fullY = 8;
    m_Progresss = (p <= 100) ? p : 100;
    m_Progresss = (m_Progresss <= 0) ? 0 : m_Progresss;
    m_YOffset = zeroY - p * (zeroY - fullY) / 100;
}

void WaterProgressInterPrivate::paint(QPainter &p)
{
    Q_Q(WaterProgressInter);

    auto rect = QRectF(q->rect());
    auto w = rect.width();
    auto h = rect.height();

    auto backgroundBrush = QBrush(qRgb(0xcb, 0xe0, 0xff));
    // draw backgroud
    QPainterPath outerPath;
    outerPath.addEllipse(rect.center(), w / 2, h / 2);
    p.fillPath(outerPath, backgroundBrush);

    // draw water
    p.drawImage(m_BackXOffset1, m_YOffset, *m_BackImage);
    p.drawImage(m_BackXOffset2, m_YOffset, *m_BackImage);
    p.drawImage(m_ForntXOffset1, m_YOffset, *m_FrontImage);
    p.drawImage(m_ForntXOffset2, m_YOffset, *m_FrontImage);
}

WaterProgress::WaterProgress(QWidget *parent): QWidget(parent)
{
    water = new WaterProgressInter(this);
    waterBoder = new WaterProgressBorder(this);

    waterBoder->raise();
    water->setProgress(90);

    updateSize(QSize(64 , 64));


    water->start();
}

void WaterProgress::updateSize(QSize size)
{
    setFixedSize(size);
    water->setFixedSize(size);
    waterBoder->setFixedSize(size);

    auto margin = 2;
    auto waterMaskRect = QRect(margin / 2, margin / 2, size.width() - margin, size.height() - margin);
    water->setMask(QRegion(waterMaskRect, QRegion::Ellipse));
}

WaterProgressBorder::WaterProgressBorder(QWidget *parent): QWidget(parent)
{

}

void WaterProgressBorder::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    auto rectF = QRectF(rect());
    auto overridePenWidth = 2;
    QPen borderPen(Qt::white, overridePenWidth);
    p.setPen(borderPen);

    auto waterMaskRect = QRectF(overridePenWidth / 2,
                                overridePenWidth / 2,
                                rectF.width() - overridePenWidth,
                                rectF.height() - overridePenWidth);
    auto center =  waterMaskRect.center();

    p.drawEllipse(center, waterMaskRect.width() / 2 ,  waterMaskRect.height() / 2);
}
