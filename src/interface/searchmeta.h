// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
