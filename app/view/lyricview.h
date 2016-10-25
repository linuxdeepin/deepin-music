/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef LYRICVIEW_H
#define LYRICVIEW_H

#include <QFrame>

class QScrollArea;
class QLabel;
class MusicInfo;

class LyricView : public QFrame
{
    Q_OBJECT
public:
    explicit LyricView(QWidget *parent = 0);

    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

    void setLyricLines(const QStringList &lines);
signals:

public slots:
//    void onMusicInfoUpdate(class MusicInfo& info);

private:
    QScrollArea *m_scroll = nullptr;
    QLabel      *m_lyric = nullptr;
};

#endif // LYRICVIEW_H
