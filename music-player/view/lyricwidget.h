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

#include <searchmeta.h>
#include "core/playlist.h"

class LyricWidgetPrivate;
class LyricWidget : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString defaultCover READ defaultCover WRITE setDefaultCover)

public:
    explicit LyricWidget(QWidget *parent = 0);
    ~LyricWidget();

    void updateUI();
    QString defaultCover() const;
    void checkHiddenSearch(QPoint mousePos);

signals:
    void toggleLyricView();
    void requestContextSearch(const QString &context);
    void changeMetaCache(const MetaPtr meta, const DMusic::SearchMeta &search);

public slots:
    void onProgressChanged(qint64 value, qint64 length);
    void onMusicPlayed(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicStop(PlaylistPtr, const MetaPtr);

    void onLyricChanged(const MetaPtr meta,
                        const DMusic::SearchMeta &search,
                        const QByteArray &lyricData);
    void onCoverChanged(const MetaPtr meta,
                        const DMusic::SearchMeta &search,
                        const QByteArray &coverData);

    void onContextSearchFinished(const QString &context,
                                 const QList<DMusic::SearchMeta> &metalist);

    void setDefaultCover(QString defaultCover);
    void onUpdateMetaCodec(const MetaPtr meta);

protected:
    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<LyricWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), LyricWidget)
};
