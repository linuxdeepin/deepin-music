/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QFrame>

#include "../core/playlist.h"

class LyricViewPrivate;
class LyricView : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString defaultCover READ defaultCover WRITE setDefaultCover)
public:
    explicit LyricView(QWidget *parent = 0);
    ~LyricView();

    void initUI();
    QString defaultCover() const;
    void checkHiddenSearch(QPoint mousePos);

signals:
    void toggleLyricView();
    void requestContextSearch(const QString &context);
    void changeMetaCache(const MusicMeta &meta);

public slots:
    void onProgressChanged(qint64 value, qint64 length);
    void onMusicPlayed(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicStop(PlaylistPtr, const MusicMeta &meta);
    void onLyricChanged(const MusicMeta &meta, const QByteArray &lyricData);
    void onCoverChanged(const MusicMeta &meta, const QByteArray &coverData);
    void setDefaultCover(QString defaultCover);

    void contextSearchFinished(const QString &context, const QList<MusicMeta> &metalist);

protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    QScopedPointer<LyricViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), LyricView)
};
