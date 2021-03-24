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

#pragma once

#include <QThread>
#include <QVector>
#include <QRandomGenerator>
#include <numeric>

#include "libdmusic_global.h"

class MetaBufferDetectorPrivate;
class LIBDMUSICSHARED_EXPORT MetaBufferDetector : public QThread
{
    Q_OBJECT
public:
    explicit MetaBufferDetector(QObject *parent = Q_NULLPTR);
    ~MetaBufferDetector();
public slots:
    void onBufferDetector(const QString &path, const QString &hash);
    void onClearBufferDetector();

signals:
    void metaBuffer(const QVector<float> &buffer, const QString &hash);

private:
    void resample(const QVector<float> &buffer, const QString &hash, bool forceQuit = false);
    int queryCacheExisted(const QString &hash);
    void run() override;
private:
//    QScopedPointer<MetaBufferDetectorPrivate> d_ptr;
//    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MetaBufferDetector)
    QString           m_curPath;
    QString           m_curHash;
    QVector<float>    m_listData;

    bool              m_stopFlag = false;
};
