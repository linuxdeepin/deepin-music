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
    explicit MetaAnalyzer(const MusicMeta &meta, DMusic::Net::Geese *geese, QObject *parent = 0);

signals:
    void searchFinished(const MusicMeta &meta, NeteaseSong song);

public slots:
    void onGetTitleResult(QList<NeteaseSong> songlist);
    void onGetAblumResult(QList<NeteaseSong> songlist);

private:
    void analyzerResults();

    DMusic::Net::Geese  *m_geese = nullptr;
    MediaMeta           m_meta;

    bool                m_titleResultGet = false;
    QList<NeteaseSong>  m_titleResult;
    bool                m_ablumResultGet = false;
    QList<NeteaseSong>  m_ablumResult;
    QTimer              m_delayTimer;
};

