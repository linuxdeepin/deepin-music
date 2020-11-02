/*
 * (c) 2017, Deepin Technology Co., Ltd. <support@deepin.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
#include "tooltips.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

#include <DUtil>
#include <dthememanager.h>
#include <DLabel>

DWIDGET_USE_NAMESPACE
class ToolTipsPrivate
{
public:
    explicit ToolTipsPrivate(ToolTips *parent) : q_ptr(parent) {}

    void setBackgroundImage(const QPixmap &srcPixmap);

    QBrush          background;
    int             radius              = 8;
    int             shadowWidth         = 20;
    QMargins        shadowMargins       = QMargins(20, 20, 20, 20);
    QColor          borderColor         = QColor(0, 0, 0, 51/*0.2 * 255*/);

    DLabel          *textLable          = nullptr;
    QFrame          *m_interFrame       = nullptr;


    ToolTips *q_ptr;
    Q_DECLARE_PUBLIC(ToolTips)
};


ToolTips::ToolTips(const QString &text, QWidget *parent)
    : QFrame(parent), d_ptr(new ToolTipsPrivate(this))
{
    DThemeManager::instance()->registerWidget(this);
    Q_D(ToolTips);

    setAttribute(Qt::WA_DeleteOnClose);
//    setWindowFlags(Qt::ToolTip | Qt::CustomizeWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
//    setMaximumWidth(_fontMinWidth);
    setObjectName("ToolTips");
    setContentsMargins(0, 0, 0, 0);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(7, 4, 7, 4);
    layout->setSpacing(0);

    d->m_interFrame = new QFrame(this);
    d->m_interFrame->setContentsMargins(0, 0, 0, 0);
    auto interlayout = new QHBoxLayout(d->m_interFrame);
    interlayout->setContentsMargins(0, 0, 0, 0);
    interlayout->setSpacing(5);
//    auto iconLabel = new QLabel;
//    iconLabel->setObjectName("TipIcon");
//    iconLabel->setFixedSize(icon.size());
//    if (icon.isNull()) {
//        iconLabel->hide();
//    } else {
//        iconLabel->setPixmap(icon);
//    }

    d->textLable = new DLabel(text);
    d->textLable->setObjectName("TipText");
    d->textLable->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
//    d->textLable->setMaximumWidth(this->width());
    d->textLable->setWordWrap(false);
    DFontSizeManager::instance()->bind(d->textLable, DFontSizeManager::T8);
    d->textLable->setForegroundRole(DPalette::ToolTipText);
//    DPalette pa_name = DApplicationHelper::instance()->palette(d->textLable);
//    pa_name.setBrush(DPalette::Text, pa_name.color(DPalette::ToolTipText));
//    pa_name.setBrush(DPalette::ToolTipText, pa_name.color(DPalette::ToolTipText));
//    d->textLable->setForegroundRole(DPalette::Text);
//    d->textLable->setForegroundRole(DPalette::ToolTipText);
//    d->textLable->setPalette(pa_name);

//    interlayout->addWidget(iconLabel, 0, Qt::AlignVCenter);
    interlayout->addWidget(d->textLable, 0, Qt::AlignVCenter);
    layout->addWidget(d->m_interFrame, 0, Qt::AlignVCenter);

    adjustSize();

//    auto *bodyShadow = new QGraphicsDropShadowEffect;
//    bodyShadow->setBlurRadius(10.0);
//    bodyShadow->setColor(QColor(0, 0, 0, 25/*0.1 * 255*/));
//    bodyShadow->setOffset(0, 2.0);
//    this->setGraphicsEffect(bodyShadow);
    hide();
//    d->textLable->hide();

    //setFixedHeight(32);
}

ToolTips::~ToolTips()
{

}

void ToolTips::enterEvent(QEvent *e)
{
    Q_UNUSED(e)
    hide();
}

QBrush ToolTips::background() const
{
    Q_D(const ToolTips);
    return d->background;
}

void ToolTips::setText(const QString text)
{
    Q_D(const ToolTips);
    d->textLable->setText(text);
    m_strText = text;
    update();
}

int ToolTips::radius() const
{
    Q_D(const ToolTips);
    return d->radius;
}

QColor ToolTips::borderColor() const
{
    Q_D(const ToolTips);
    return d->borderColor;
}

//void ToolTips::setBackground(QBrush background)
//{
//    Q_D(ToolTips);
//    d->background = background;
//}

void ToolTips::setRadius(int radius)
{
    Q_D(ToolTips);
    d->radius = radius;
}

//void ToolTips::setBorderColor(QColor borderColor)
//{
//    Q_D(ToolTips);
//    d->borderColor = borderColor;
//}

//void ToolTips::pop(QPoint center)
//{
//    //Q_D(ToolTips);
//    this->show();
//    center = center - QPoint(width() / 2, height() / 2);
//    this->move(center);

////    auto topOpacity = new QGraphicsOpacityEffect(d->m_interFrame);
////    topOpacity->setOpacity(1);
////    d->m_interFrame->setGraphicsEffect(topOpacity);

////    QPropertyAnimation *animation4 = new QPropertyAnimation(topOpacity, "opacity");
//////    animation4->setEasingCurve(QEasingCurve::InCubic);
////    animation4->setDuration(2000);
////    animation4->setStartValue(0);
////    animation4->setKeyValueAt(0.25, 1);
////    animation4->setKeyValueAt(0.5, 1);
////    animation4->setKeyValueAt(0.75, 1);
////    animation4->setEndValue(0);
////    animation4->start();
////    animation4->connect(animation4, &QPropertyAnimation::finished,
////                        animation4, &QPropertyAnimation::deleteLater);
////    animation4->connect(animation4, &QPropertyAnimation::finished,
////    this, [ = ]() {
////        d->m_interFrame->setGraphicsEffect(nullptr);
////        this->hide();
////    });
//}

#ifdef _OLD
void ToolTips::paintEvent(QPaintEvent *)
{
    Q_D(ToolTips);
//    QFrame::paintEvent(e);
//    return;
//    bool outer = true;

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    auto radius = d->radius;
    auto penWidthf = 1.0;
//    auto background =  d->background;
    const QPalette pal = QGuiApplication::palette();//this->palette();
    QColor background = pal.color(QPalette::ToolTipBase);
    DPalette pa_name = DApplicationHelper::instance()->palette(d->textLable);
    pa_name.setBrush(DPalette::Text, pa_name.color(DPalette::ToolTipText));
    pa_name.setBrush(DPalette::ToolTipText, pa_name.color(DPalette::ToolTipText));
    d->textLable->setForegroundRole(DPalette::Text);
    d->textLable->setForegroundRole(DPalette::ToolTipText);
    d->textLable->setPalette(pa_name);
    auto borderColor = d->borderColor;
    auto margin = 2.0;
    auto shadowMargins = QMarginsF(margin, margin, margin, margin);

//    QPainterPath frame;
//    frame.addRect(rect().marginsRemoved(QMargins(1, 1, 1, 1)));
//    painter.strokePath(frame, QPen(Qt::red));

    // draw background
    auto backgroundRect = QRectF(rect()).marginsRemoved(shadowMargins);
    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(backgroundRect, radius, radius);
    painter.fillPath(backgroundPath, background);

    // draw border
    QPainterPath borderPath;
    QRectF borderRect = QRectF(rect());
    auto borderRadius = radius;
    QMarginsF borderMargin(penWidthf / 2, penWidthf / 2, penWidthf / 2, penWidthf / 2);
//    if (outer) {
    borderRadius += penWidthf / 2;
    borderRect = borderRect.marginsAdded(borderMargin).marginsRemoved(shadowMargins);
//    } else {
//        borderRadius -= penWidthf / 2;
//        borderRect = borderRect.marginsRemoved(borderMargin).marginsRemoved(shadowMargins);
//    }
    borderPath.addRoundedRect(borderRect, borderRadius, borderRadius);
    QPen borderPen(borderColor);
    borderPen.setWidthF(penWidthf);
    painter.strokePath(borderPath, borderPen);
}

#else
void ToolTips::paintEvent(QPaintEvent *)
{
    Q_D(ToolTips);
    QPainter pt(this);
    pt.setRenderHint(QPainter::Antialiasing);

//    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [=]{
//        DPalette pa_name = DApplicationHelper::instance()->palette(d->textLable);
//        pa_name.setBrush(DPalette::Text, pa_name.color(DPalette::ToolTipText));
//        pa_name.setBrush(DPalette::ToolTipText, pa_name.color(DPalette::ToolTipText));
//        d->textLable->setForegroundRole(DPalette::Text);
//        d->textLable->setForegroundRole(DPalette::ToolTipText);
//        d->textLable->setPalette(pa_name);
//    });

    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        pt.setPen(QColor(0, 0, 0, 10));
        pt.setBrush(QBrush(QColor(247, 247, 247, 220)));
    } else if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        pt.setPen(QColor(255, 255, 255, 10));
        pt.setBrush(QBrush(QColor(42, 42, 42, 220)));
    } else {
        pt.setPen(QColor(0, 0, 0, 10));
        pt.setBrush(QBrush(QColor(247, 247, 247, 220)));
    }

    QRect rect = this->rect();
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);
    QPainterPath painterPath;
    painterPath.addRoundedRect(rect, d->radius, d->radius);
    pt.drawPath(painterPath);

}
#endif

void ToolTips::resizeEvent(QResizeEvent *ev)
{
//    resetSize();
//    update();
    return QWidget::resizeEvent(ev);
}

//void ToolTips::resetSize(const int maxWidth)
//{
//    Q_D(ToolTips);
//    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
//    QFontMetrics fm(font);
//    auto w = fm.boundingRect(d->textLable->text()).width();

//    if (w >= maxWidth - 14) {
//        d->textLable->setWordWrap(true);
//        this->setFixedWidth(maxWidth);
//        d->textLable->setFixedWidth(maxWidth - 14);
//    }
//}


