// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTSFILTER_H
#define EVENTSFILTER_H

#include <QObject>

class EventsFilterPrivate;
class EventsFilter : public QObject
{
    Q_OBJECT
public:
    EventsFilter(QObject *parent = nullptr);

    Q_INVOKABLE void setEnabled(const bool &enabled);

signals:
    void keyFiltered(int key,int Modifier);
    void mousePress(int x,int y);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    EventsFilterPrivate *m_data;
};

#endif // KEYSFILTER_H
