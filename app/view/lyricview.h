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
#include <QScopedPointer>

class MusicMeta;
class LyricViewPrivate;

class LyricView : public QFrame
{
    Q_OBJECT
public:
    explicit LyricView(QWidget *parent = 0);
    ~LyricView();

    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

    void setLyricLines(QString lines);
signals:

public slots:
    void onProgressChanged(qint64 value, qint64 length);
    void onLyricChanged(const MusicMeta &info, const QString &lyricPath);
    void onCoverChanged(const MusicMeta &info, const QString &coverPath);

private:
    void adjustLyric();
    QScopedPointer<LyricViewPrivate>  d;
};

#endif // LYRICVIEW_H
