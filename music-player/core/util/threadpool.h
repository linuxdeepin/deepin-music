/*
 * Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
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

#pragma once

#include <QThread>

#include <util/singleton.h>

class ThreadPool : public QObject, public DMusic::DSingleton<ThreadPool>
{
    Q_OBJECT
public:
    explicit ThreadPool(QObject *parent = 0);
    ~ThreadPool();

    QThread *newThread();
    void moveToNewThread(QObject *obj);
    void manager(QThread *thread);
    void quitAll();

private:
    friend class DMusic::DSingleton<ThreadPool>;

    QList<QThread *> m_pool;
};

