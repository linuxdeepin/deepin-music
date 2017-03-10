/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QFrame>

#include "../../core/playlist.h"
#include <dpicturesequenceview.h>

class TitelEdit;
class PlayListItem : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString animationPrefix READ animationPrefix WRITE setAnimationPrefix)
    Q_PROPERTY(QString highlightAnimationPrefix READ highlightAnimationPrefix WRITE setHighlightAnimationPrefix)
public:
    explicit PlayListItem(PlaylistPtr playlist, QWidget *parent = 0);
    inline PlaylistPtr data() {return m_data;}

    void setActive(bool active);
    void setPlay(bool isPaly);

    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    QString animationPrefix() const
    {
        return m_animationPrefix;
    }

    QString highlightAnimationPrefix() const
    {
        return m_highlightAnimationPrefix;
    }

signals:
    void rename(const QString &newNameA);
    void remove();
    void playall(PlaylistPtr playlist);

public slots:
    void onDelete();
    void onRename();

    void setAnimationPrefix(QString animationPrefix)
    {
        m_animationPrefix = animationPrefix;
    }

    void setHighlightAnimationPrefix(QString highlightAnimationPrefix)
    {
        m_highlightAnimationPrefix = highlightAnimationPrefix;
    }

private:
    Dtk::Widget::DPictureSequenceView  *playingAnimation;
    TitelEdit      *m_titleedit = nullptr;
    PlaylistPtr    m_data;
    QString m_animationPrefix;
    QString m_highlightAnimationPrefix;
};

#endif // PLAYLISTITEM_H
