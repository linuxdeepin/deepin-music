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

#include <QString>
#include <QList>
#include <QObject>

namespace DMusic {

struct SearchAlbum {
    int     id = 0;
    QString name;
    QString coverUrl;
};

struct SearchArtist {
    int     id = 0;
    QString name;
    QString avatarUrl;
};

struct SearchMeta {
    QString                 id;
    QString                 name;
    SearchAlbum             album;
    QList<SearchArtist>     artists;
    int                     length = 0;

    SearchMeta() {}
    SearchMeta(const QString &id): id(id) {}
};

}

Q_DECLARE_METATYPE(DMusic::SearchMeta)
Q_DECLARE_METATYPE(QList<DMusic::SearchMeta>)
