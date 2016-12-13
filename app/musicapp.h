/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MUSICAPP_H
#define MUSICAPP_H

#include <QObject>
#include <QScopedPointer>

#include "presenter/presenter.h"

class MusicAppPrivate;

class MusicApp : public QObject
{
    Q_OBJECT
public:
    ~MusicApp();

    static MusicApp *instance()
    {
        static auto s_app = new MusicApp;
        return s_app;
    }

    static QString configPath();
    static QString cachePath();

    void init();

public slots:
    void onDataPrepared();

private:
    explicit MusicApp(QObject *parent = 0);
    QScopedPointer<MusicAppPrivate> d;
};

#endif // MUSICAPP_H
