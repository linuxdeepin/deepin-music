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

class ViewPresenter : public QObject
{
    Q_OBJECT
public:
    explicit ViewPresenter(QObject *parent = 0);

signals:
    void showImportDialog(const QUrl &path);

    void showImportView();
    void showMusiclistView();
    void toggleLayricView();

    void hidePlaylist();
    void showPlaylist();
};
