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

#ifndef FFMPEGDYNAMICINSTANCE_H
#define FFMPEGDYNAMICINSTANCE_H

#include <QObject>
#include <QLibrary>
#include <QMap>

class FfmpegDynamicInstance : public QObject
{
    Q_OBJECT
public:
    static FfmpegDynamicInstance *VlcFunctionInstance();
    QFunctionPointer resolveSymbol(const char *symbol, bool bffmpeg = false);
signals:

public slots:

private:
    explicit FfmpegDynamicInstance(QObject *parent = nullptr);
    ~FfmpegDynamicInstance();
    bool loadVlcLibrary();
    /**
     * @brief libPath get absolutely library path
     * @param strlib library name
     * @return
     */
    QString libPath(const QString &strlib);
    QLibrary libdavcode;
    QLibrary libddformate;

    QMap<QString, QFunctionPointer> m_funMap;
};

#endif // FFMPEGDYNAMICINSTANCE_H
