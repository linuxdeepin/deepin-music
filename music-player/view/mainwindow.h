/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <DWindow>
#include <QScopedPointer>

#include "../core/playlist.h"
#include "widget/thinwindow.h"

class Presenter;
class MusicListWidget;
class MainWindowPrivate;
class MainWindow : public ThinWindow
{
    Q_OBJECT
    Q_PROPERTY(QString coverBackground READ coverBackground WRITE setCoverBackground)

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void initUI();

    void initMusiclist(PlaylistPtr allmusic, PlaylistPtr last);
    void initPlaylist(QList<PlaylistPtr > playlists, PlaylistPtr last);
    void initFooter(PlaylistPtr current, int mode);
    void binding(Presenter *presenter);

    QString coverBackground() const;

signals:
    void addPlaylist(bool editmode);
    void importSelectFiles(const QStringList &filelist);

public slots:
    void setCoverBackground(QString coverBackground);
    void onSelectImportFiles();
    void onCurrentPlaylistChanged(PlaylistPtr playlist);
    void toggleLyricView();
    void togglePlaylist();
    void showLyricView();
    void showMusicListView();
    void showImportView();
    void showTips(QPixmap icon, QString text);
    void setPlaylistVisible(bool visible);

protected:
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    void changeToMusicListView(bool keepPlaylist);
    void initMenu();
    void disableControl();
    void updateViewname(const QString &vm);

    QScopedPointer<MainWindowPrivate> d;
};

extern const QString s_PropertyViewname;
extern const QString s_PropertyViewnameLyric;
