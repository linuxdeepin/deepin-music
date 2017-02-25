/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
    Q_PROPERTY(QString animationPrefix READ animationPrefix WRITE setAnimationPrefix)
    Q_PROPERTY(QString highlightAnimationPrefix READ highlightAnimationPrefix WRITE setHighlightAnimationPrefix)

public:
    int                                 tialWidth = -1;

    explicit MusicItemDelegatePrivate(MusicItemDelegate *parent = 0);

    inline QColor foreground(int col, const QStyleOptionViewItem &option) const;
    inline int timePropertyWidth(const QStyleOptionViewItem &option) const;
    void setActiveAnimationPrefix(QString prefix) const;

    QColor textColor() const;
    QColor titleColor() const;
    QColor highlightText() const;
    QColor background() const;
    QColor alternateBackground() const;
    QColor highlightedBackground() const;
    QString animationPrefix() const;
    QString highlightAnimationPrefix() const;

public slots:
    void setTextColor(QColor textColor);
    void setTitleColor(QColor titleColor);
    void setHighlightText(QColor highlightText);
    void setBackground(QColor background);
    void setAlternateBackground(QColor alternateBackground);
    void setHighlightedBackground(QColor highlightedBackground);
    void setAnimationPrefix(QString animationPrefix);
    void setHighlightAnimationPrefix(QString highlightAnimationPrefix);

private:
    QColor m_textColor;
    QColor m_numberColor;
    QColor m_highlightText;
    QColor m_background;
    QColor m_alternateBackground;
    QColor m_highlightedBackground;
    QString m_aimationPrefix;
    QString m_highlightAnimationPrefix;

    MusicItemDelegate *q_ptr;
    Q_DECLARE_PUBLIC(MusicItemDelegate)
};
