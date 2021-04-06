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

#include "filter.h"

#include <QDebug>

#include <QEvent>
#include <QCursor>
#include <DWidget>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

#include <DUtil>

//HoverFilter::HoverFilter(QObject *parent) : QObject(parent)
//{

//}

//bool HoverFilter::eventFilter(QObject *obj, QEvent *event)
//{
//    switch (event->type()) {
//    case QEvent::Enter: {
//        auto w = qobject_cast<QWidget *>(obj);
//        w->setCursor(QCursor(Qt::PointingHandCursor));
////        qDebug() << "set cursor" << w << w->cursor();
////        QApplication::setOverrideCursor(Qt::PointingHandCursor);
//        return QObject::eventFilter(obj, event);
//    }
//    case QEvent::Leave: {
//        auto w = qobject_cast<QWidget *>(obj);
////        qDebug() << "unset cursor" << w;
//        w->unsetCursor();
//        QApplication::restoreOverrideCursor();
//        return QObject::eventFilter(obj, event);
//    }
//    default:
//        return QObject::eventFilter(obj, event);
//    }
//}

HintFilter::HintFilter(QObject *parent) : QObject(parent)
{
    m_delayShowTimer = new QTimer(this);
    m_delayShowTimer->setInterval(1000);
    connect(m_delayShowTimer, &QTimer::timeout, this, [ = ]() {
        if (m_parentWidget) {
            auto hint = m_parentWidget->property("HintWidget").value<QWidget *>();
            showHint(hint);
        }
        m_delayShowTimer->stop();
    });
}

HintFilter::~HintFilter()
{

}

//void HintFilter::hideAll()
//{
//    Q_D(HintFilter);
//    if (d->hintWidget) {
//        d->hintWidget->hide();
//        d->delayShowTimer->stop();
//    }
//}

bool HintFilter::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter: {
        if (m_hintWidget) {
            m_hintWidget->hide();
        }

        auto w = qobject_cast<QWidget *>(obj);
        m_parentWidget = w;
        if (!w) {
            break;
        }

        m_hintWidget = w->property("HintWidget").value<QWidget *>();
        if (!m_hintWidget) {
            break;
        }

        m_delayShowTimer->stop();

        bool nodelayshow = m_hintWidget->property("NoDelayShow").toBool();
        if (nodelayshow) {
            showHint(m_hintWidget);
        } else {
            m_delayShowTimer->start();
        }

        m_parentWidget->setCursor(QCursor(Qt::ArrowCursor));
        break;
    }
    case QEvent::Leave: {
        if (m_hintWidget) {
            if (!m_hintWidget->property("DelayHide").toBool()) {
                m_hintWidget->hide();
                m_delayShowTimer->stop();
            } else {
                QMetaObject::invokeMethod(m_hintWidget, "deleyHide", Qt::DirectConnection);
            }

        }
        auto w = qobject_cast<QWidget *>(obj);
        if (w) {
            w->unsetCursor();
        }

        break;
    }
    case QEvent::MouseButtonPress:
        if (m_hintWidget) {
            if (!m_hintWidget->property("_dm_keep_on_click").toBool()) {
                m_hintWidget->hide();
            }
            m_delayShowTimer->stop();
        }

        break;
    default:
        break;
    }
    return QObject::eventFilter(obj, event);
}

void HintFilter::showHitsFor(QWidget *w, QWidget *hint)
{
    if (m_hintWidget) {
        m_hintWidget->hide();
    }

    m_parentWidget = w;
    if (!w) {
        return;
    }

    m_hintWidget = hint;
    if (!m_hintWidget) {
        return;
    }

    m_delayShowTimer->stop();

    showHint(hint);

    m_hintWidget->setCursor(QCursor(Qt::ArrowCursor));
}

void HintFilter::showHint(QWidget *hint)
{
    if (!m_parentWidget) {
        return;
    }
    auto w = m_parentWidget;
    if (m_hintWidget && m_hintWidget != hint) {
        m_hintWidget->hide();
    }
    m_hintWidget = hint;
    if (!m_hintWidget) {
        return;
    }


    DUtil::TimerSingleShot(10, [w, this]() {
        auto centerPos = w->mapToGlobal(w->rect().center());
        m_hintWidget->show();
        m_hintWidget->adjustSize();

        auto sz = m_hintWidget->size();
        centerPos.setX(centerPos.x()  - sz.width() / 2);
        centerPos.setY(centerPos.y() - 32 - sz.height());
        centerPos = m_hintWidget->mapFromGlobal(centerPos);
        centerPos = m_hintWidget->mapToParent(centerPos);
        m_hintWidget->move(centerPos);
        m_hintWidget->raise();
    });
}

//HoverShadowFilter::HoverShadowFilter(QObject *parent): QObject(parent)
//{

//}

//bool HoverShadowFilter::eventFilter(QObject *obj, QEvent *event)
//{
//    switch (event->type()) {
//    case QEvent::Enter: {
//        auto w = qobject_cast<QWidget *>(obj);
//        auto shadow = new QGraphicsDropShadowEffect(w);
//        shadow->setBlurRadius(8);
//        shadow->setOffset(0, 0);
//        shadow->setColor(Qt::white);
//        w->setGraphicsEffect(shadow);
//        w->setCursor(QCursor(Qt::PointingHandCursor));
//        return QObject::eventFilter(obj, event);
//    }
//    case QEvent::Leave: {
//        auto w = qobject_cast<QWidget *>(obj);
//        w->graphicsEffect()->deleteLater();
//        w->setGraphicsEffect(nullptr);
//        w->unsetCursor();
//        return QObject::eventFilter(obj, event);
//    }
//    default:
//        return QObject::eventFilter(obj, event);
//    }
//}
