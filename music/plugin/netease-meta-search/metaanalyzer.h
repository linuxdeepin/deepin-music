/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
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

#include <QObject>
#include <QTimer>

#include <mediameta.h>
#include <net/geese.h>

#include <searchmeta.h>

class MetaAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit MetaAnalyzer(const MetaPtr meta, DMusic::Net::Geese *geese, QObject *parent = 0);

signals:
    void searchFinished(const MetaPtr meta, DMusic::SearchMeta song);

public slots:
    void onGetTitleResult(QList<DMusic::SearchMeta> songlist);
    void onGetAblumResult(QList<DMusic::SearchMeta> songlist);

private:
    void analyzerResults();

    DMusic::Net::Geese  *m_geese = nullptr;
    MetaPtr             m_meta;

    bool                m_titleResultGet = false;
    QList<DMusic::SearchMeta>   m_titleResult;

    bool                m_ablumResultGet = false;
    QList<DMusic::SearchMeta>   m_ablumResult;
    QTimer              m_delayTimer;
};

