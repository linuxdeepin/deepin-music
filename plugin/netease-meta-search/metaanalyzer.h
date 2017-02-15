/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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

