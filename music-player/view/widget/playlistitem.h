/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QFrame>

#include "../../core/playlist.h"
#include <dpicturesequenceview.h>

class TitelEdit;
class PlayListItem : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString playingIcon READ playingIcon WRITE setPlayingIcon)
    Q_PROPERTY(QString highlightPlayingIcon READ highlightPlayingIcon WRITE setHighlightPlayingIcon)
public:
    explicit PlayListItem(PlaylistPtr playlist, QWidget *parent = 0);
    inline PlaylistPtr data() {return m_data;}

    void setActive(bool active);
    void setPlay(bool isPaly);

    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    QString playingIcon() const
    {
        return m_playingIcon;
    }

    QString highlightPlayingIcon() const
    {
        return m_highlightPlayingIcon;
    }

signals:
    void rename(const QString &newNameA);
    void remove();
    void playall(PlaylistPtr playlist);

public slots:
    void onDelete();
    void onRename();

    void setPlayingIcon(QString playingIcon)
    {
        m_playingIcon = playingIcon;
    }

    void setHighlightPlayingIcon(QString highlightPlayingIcon)
    {
        m_highlightPlayingIcon = highlightPlayingIcon;
    }

private:
//    Dtk::Widget::DPictureSequenceView  *playingAnimation;
    TitelEdit      *m_titleedit = nullptr;
    PlaylistPtr    m_data;
    QString m_playingIcon;
    QString m_highlightPlayingIcon;
};

#endif // PLAYLISTITEM_H
