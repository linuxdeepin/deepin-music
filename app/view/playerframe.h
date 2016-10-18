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
#include <QAbstractListModel>

class MusicInfo;
class MusicListInfo;

class MusicListWidget;
class PlayerFramePrivate;
class PlayerFrame : public Dtk::Widget::DWindow
{
    Q_OBJECT
public:
    explicit PlayerFrame(QWidget *parent = 0);
    ~PlayerFrame();

    MusicListWidget *musicList();

    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
signals:

public slots:
    void onMusicListChanged(const MusicListInfo&);

private:
    QScopedPointer<PlayerFramePrivate> d;
};

#endif // PLAYERFRAME_H
