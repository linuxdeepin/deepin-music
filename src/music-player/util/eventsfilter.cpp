// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventsfilter.h"

#include <QKeyEvent>
#include <QDebug>

class EventsFilterPrivate
{
public:
    EventsFilterPrivate(EventsFilter *parent)
        : m_parent(parent)
    {
        m_keyStatus.insert(Qt::Key_Space, true);
        m_keyStatus.insert(Qt::Key_A, true);
    }

private:
    friend class EventsFilter;
    EventsFilter                  *m_parent;
    QMap<int, bool>                m_keyStatus;
    bool                           m_filterFlag = true;
};

EventsFilter::EventsFilter(QObject *parent)
    : QObject(parent), m_data(new EventsFilterPrivate(this))
{

}

void EventsFilter::setEnabled(const bool &enabled)
{
    m_data->m_filterFlag = enabled;
}

bool EventsFilter::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (m_data->m_filterFlag && m_data->m_keyStatus.contains(keyEvent->key())
                && m_data->m_keyStatus[keyEvent->key()]) {
            if (event->type() == QEvent::KeyPress) {
                emit keyFiltered(keyEvent->key(), static_cast<int>(keyEvent->modifiers()));
                if (keyEvent->key() == Qt::Key_Space) {
                    return true;
                }
            }
        }
    }
    break;
    case QEvent::QEvent::MouseButtonPress: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        emit mousePress(mouseEvent->globalX(), mouseEvent->globalY());
    }
    break;
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}
