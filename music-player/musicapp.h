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

#include <util/singleton.h>

class MprisPlayer;
class MusicAppPrivate;
class MusicApp : public QObject, public DMusic::DSingleton<MusicApp>
{
    Q_OBJECT
public:
    ~MusicApp();

    void init();
    void initMpris(MprisPlayer *mprisPlayer);
    void openUri(const QUrl &uri);

    void triggerShortcutAction(const QString &optKey);
public slots:
    void onDataPrepared();
    void onQuit();
    void onRaise();

private:
    MusicApp(QObject *parent = nullptr);
    friend class DMusic::DSingleton<MusicApp>;
    QScopedPointer<MusicAppPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusicApp)
};
