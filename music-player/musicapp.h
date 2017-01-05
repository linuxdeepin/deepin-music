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
#include <QScopedPointer>

#include "core/util/singleton.h"

class MprisPlayer;
class MusicAppPrivate;
class MusicApp : public QObject, public Singleton<MusicApp>
{
    Q_OBJECT
public:
    ~MusicApp();

    static QString configPath();
    static QString cachePath();

    void init();
    void initMpris(MprisPlayer* mprisPlayer);

    QWidget *hackFrame();
public slots:
    void onDataPrepared();
    void onQuit();
    void onRaise();

private:
    friend class Singleton<MusicApp>;
    explicit MusicApp(QObject *parent = 0);
    QScopedPointer<MusicAppPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusicApp)
};
