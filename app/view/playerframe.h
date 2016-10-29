/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PLAYERFRAME_H
#define PLAYERFRAME_H

#include <DWindow>
#include <QScopedPointer>

DWIDGET_USE_NAMESPACE

class Playlist;
class MusicMeta;
class AppPresenter;
class MusicListWidget;
class PlayerFramePrivate;
class PlayerFrame : public DWindow
{
    Q_OBJECT
public:
    explicit PlayerFrame(QWidget *parent = 0);
    ~PlayerFrame();

    void initMusiclist(QSharedPointer<Playlist> allmusic, QSharedPointer<Playlist> last);
    void initPlaylist(QList<QSharedPointer<Playlist> > playlists, QSharedPointer<Playlist> last);
    void initFooter(QSharedPointer<Playlist> favlist, QSharedPointer<Playlist> current, int mode);
    void binding(AppPresenter *presenter);

    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
signals:
    void importSelectFiles(const QStringList &filelist);

public slots:
    void onSelectImportFiles();

private:
    void initMenu();

    // disable control
    void disableControl();

    QScopedPointer<PlayerFramePrivate> d;
};

#endif // PLAYERFRAME_H
