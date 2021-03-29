/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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

ToolTips::ToolTips(const QString &text, QWidget *parent)
    : QFrame(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
//    setWindowFlags(Qt::ToolTip | Qt::CustomizeWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
//    setMaximumWidth(_fontMinWidth);
    setObjectName("ToolTips");
    setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(7, 4, 7, 4);
    layout->setSpacing(0);

    m_interFrame = new QFrame(this);
    m_interFrame->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *interlayout = new QHBoxLayout(m_interFrame);
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

    textLable = new DLabel(text);
    textLable->setObjectName("TipText");
    textLable->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
//    d->textLable->setMaximumWidth(this->width());
    textLable->setWordWrap(false);
    DFontSizeManager::instance()->bind(textLable, DFontSizeManager::T8);
    textLable->setForegroundRole(DPalette::ToolTipText);
//    DPalette pa_name = DApplicationHelper::instance()->palette(d->textLable);
//    pa_name.setBrush(DPalette::Text, pa_name.color(DPalette::ToolTipText));
//    pa_name.setBrush(DPalette::ToolTipText, pa_name.color(DPalette::ToolTipText));
//    d->textLable->setForegroundRole(DPalette::Text);
//    d->textLable->setForegroundRole(DPalette::ToolTipText);
//    d->textLable->setPalette(pa_name);

//    interlayout->addWidget(iconLabel, 0, Qt::AlignVCenter);
    interlayout->addWidget(textLable, 0, Qt::AlignVCenter);
    layout->addWidget(m_interFrame, 0, Qt::AlignVCenter);

    adjustSize();

    QGraphicsDropShadowEffect *bodyShadow = new QGraphicsDropShadowEffect(this);
    bodyShadow->setBlurRadius(10.0);
    bodyShadow->setColor(QColor(0, 0, 0, 25/*0.1 * 255*/));
    bodyShadow->setOffset(0, 2.0);
//    this->setGraphicsEffect(bodyShadow);
    hide();
//    d->textLable->hide();

    //setFixedHeight(32);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &ToolTips::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());
}

ToolTips::~ToolTips()
{

}

void ToolTips::enterEvent(QEvent *e)
{
    Q_UNUSED(e)
    hide();
}

void ToolTips::setText(const QString &text)
{
    textLable->setText(text);
    m_strText = text;
    update();
}

void ToolTips::setThemeType(int type)
{
    if (DGuiApplicationHelper::DarkType == type) {
        m_pen = QPen(QColor(255, 255, 255, 10));
        m_brush = QBrush(QBrush(QColor(42, 42, 42, 220)));
    } else {
        m_pen = QPen(QColor(0, 0, 0, 10));
        m_brush = QBrush(QColor(247, 247, 247, 220));
    }

    this->update();
}

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

void ToolTips::paintEvent(QPaintEvent *)
{
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

    pt.setPen(m_pen);
    pt.setBrush(m_brush);

    QRect rect = this->rect();
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);
    QPainterPath painterPath;
    painterPath.addRoundedRect(rect, radius, radius);
    pt.drawPath(painterPath);

}


