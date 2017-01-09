/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef LYRICSERVICE_H
#define LYRICSERVICE_H

#include <QObject>

#include "util/singleton.h"
#include <musicmeta.h>
#include <geese.h>

class MusicMeta;
class LyricService : public QObject, public Singleton<LyricService>
{
    Q_OBJECT

    friend class Singleton<LyricService>;
public:
    explicit LyricService(QObject *parent = 0);

    static QUrl coverUrl(const MusicMeta &info);
    static QByteArray coverData(const MusicMeta &info);
    static QByteArray lyricData(const MusicMeta &info);

signals:
    void lyricSearchFinished(const MusicMeta &, const QByteArray &lyricData);
    void coverSearchFinished(const MusicMeta &, const QByteArray &coverData);
    void contextSearchFinished(const QString &context, const MusicMetaList &metalist);

public slots:
    void searchMeta(const MusicMeta &info);
    void searchContext(const QString &context);
    void onChangeMetaCache(const MusicMeta &meta);

private:
    void loadMetaSearchEnginePlugin();

    int searchCacheLyric(const MusicMeta &info);
    int searchCacheCover(const MusicMeta &info);


    DMusic::Net::Geese       *m_geese = nullptr;
};

#endif // LYRICSERVICE_H
