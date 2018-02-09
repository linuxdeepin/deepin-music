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

#pragma once

#include <QModelIndex>
#include <QWidget>

class PictureSequenceView;
class QStyleOptionViewItem;
class MusicItemDelegate;
class MusicItemDelegatePrivate : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(QColor titleColor READ titleColor WRITE setTitleColor)
    Q_PROPERTY(QColor highlightText READ highlightText WRITE setHighlightText)
    Q_PROPERTY(QColor background READ background WRITE setBackground)
    Q_PROPERTY(QColor alternateBackground READ alternateBackground WRITE setAlternateBackground)
    Q_PROPERTY(QColor highlightedBackground READ highlightedBackground WRITE setHighlightedBackground)
    Q_PROPERTY(QString playingIcon READ playingIcon WRITE setPlayingIcon)
    Q_PROPERTY(QString highlightPlayingIcon READ highlightPlayingIcon WRITE setHighlightPlayingIcon)

public:
    int                                 tialWidth = -1;

    explicit MusicItemDelegatePrivate(MusicItemDelegate *parent = 0);

    inline QColor foreground(int col, const QStyleOptionViewItem &option) const;
    inline int timePropertyWidth(const QStyleOptionViewItem &option) const;

    QColor textColor() const;
    QColor titleColor() const;
    QColor highlightText() const;
    QColor background() const;
    QColor alternateBackground() const;
    QColor highlightedBackground() const;
    QString playingIcon() const;
    QString highlightPlayingIcon() const;

public slots:
    void setTextColor(QColor textColor);
    void setTitleColor(QColor titleColor);
    void setHighlightText(QColor highlightText);
    void setBackground(QColor background);
    void setAlternateBackground(QColor alternateBackground);
    void setHighlightedBackground(QColor highlightedBackground);
    void setPlayingIcon(QString playingIcon);
    void setHighlightPlayingIcon(QString highlightPlayingIcon);

private:
    QColor m_textColor;
    QColor m_numberColor;
    QColor m_highlightText;
    QColor m_background;
    QColor m_alternateBackground;
    QColor m_highlightedBackground;
    QString m_aimationPrefix;
    QString m_highlightPlayingIcon;

    MusicItemDelegate *q_ptr;
    Q_DECLARE_PUBLIC(MusicItemDelegate)
};
