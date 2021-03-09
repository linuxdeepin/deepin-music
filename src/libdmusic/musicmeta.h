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

//#include <QObject>
//#include <QString>
//#include <QStringList>
//#include <QMap>
//#include <QTime>

//inline QString lengthString(qint64 length)
//{
//    length = length / 1000;
//    QTime t(static_cast<int>(length / 3600), length % 3600 / 60, length % 60);
//    return t.toString("mm:ss");
//}

//inline QString sizeString(qint64 sizeByte)
//{
//    QString text;
//    if (sizeByte < 1024) {
//        text.sprintf("%.1fB", sizeByte / 1.0);
//        return text;
//    }
//    if (sizeByte < 1024 * 1024) {
//        text.sprintf("%.1fK", sizeByte / 1024.0);
//        return text;
//    }
//    if (sizeByte < 1024 * 1024 * 1024) {
//        text.sprintf("%.1fM", sizeByte / 1024.0 / 1024.0);
//        return text;
//    }
//    text.sprintf("%.1fG", sizeByte / 1024.0 / 1024.0 / 1024.0);
//    return text;
//}

