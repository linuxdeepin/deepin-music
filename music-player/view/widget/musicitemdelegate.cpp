/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musicitemdelegate.h"

#include <QDebug>
#include <QPainter>
#include <QStandardItemModel>

#include <dthememanager.h>
#include <musicmeta.h>

#include "musiclistview.h"
#include "musicitemdelegate_p.h"

DWIDGET_USE_NAMESPACE

const int MusicItemLeftMargin = 15;
const int MusicItemRightMargin = 20;
const int MusicItemNumberMargin = 10;

MusicItemDelegatePrivate::MusicItemDelegatePrivate(MusicItemDelegate *parent):
    QWidget(nullptr), q_ptr(parent)
{
    setObjectName("MusicItem");

    playingAnimation = new Dtk::Widget::DPictureSequenceView;
    QStringList urls;
    auto urlTemp = QString(":/light/animation/playing/%1.png");
    for (int i = 0; i < 94; ++i) {
        urls << urlTemp.arg(i);
    }
    playingAnimation->setPictureSequence(urls);
    playingAnimation->play();

    D_THEME_INIT_WIDGET(MusicItem);
}

QColor MusicItemDelegatePrivate::textColor() const
{
    return m_textColor;
}
QColor MusicItemDelegatePrivate::titleColor() const
{
    return m_numberColor;
}
QColor MusicItemDelegatePrivate::highlightText() const
{
    return m_highlightText;
}
QColor MusicItemDelegatePrivate::background() const
{
    return m_background;
}
QColor MusicItemDelegatePrivate::alternateBackground() const
{
    return m_alternateBackground;
}
QColor MusicItemDelegatePrivate::highlightedBackground() const
{
    return m_highlightedBackground;
}
void MusicItemDelegatePrivate::setTextColor(QColor textColor)
{
    m_textColor = textColor;
}
void MusicItemDelegatePrivate::setTitleColor(QColor numberColor)
{
    m_numberColor = numberColor;
}
void MusicItemDelegatePrivate::setHighlightText(QColor highlightText)
{
    m_highlightText = highlightText;
}
void MusicItemDelegatePrivate::setBackground(QColor background)
{
    m_background = background;
}
void MusicItemDelegatePrivate::setAlternateBackground(QColor alternateBackground)
{
    m_alternateBackground = alternateBackground;
}
void MusicItemDelegatePrivate::setHighlightedBackground(QColor highlightedBackground)
{
    m_highlightedBackground = highlightedBackground;
}

QColor MusicItemDelegatePrivate::foreground(int col, const QStyleOptionViewItem &option) const
{
    if (option.state & QStyle::State_Selected) {
        return highlightText();
    }

    auto emCol = static_cast<MusicItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicItemDelegate::Number:
    case MusicItemDelegate::Artist:
    case MusicItemDelegate::Album:
    case MusicItemDelegate::Length:
        return textColor();
    case MusicItemDelegate::Title:
        return titleColor();
    case MusicItemDelegate::ColumnButt:
        break;
    }
    return textColor();
}

static inline QFlags<Qt::AlignmentFlag> alignmentFlag(int col)
{
    auto emCol = static_cast<MusicItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicItemDelegate::Number:
    case MusicItemDelegate::Title:
    case MusicItemDelegate::Artist:
    case MusicItemDelegate::Album:
        return (Qt::AlignLeft | Qt::AlignVCenter);
    case MusicItemDelegate::Length:
        return (Qt::AlignRight | Qt::AlignVCenter);
    case MusicItemDelegate::ColumnButt:
        break;
    }
    return (Qt::AlignLeft | Qt::AlignVCenter);;
}

static inline QRect colRect(int col, const QStyleOptionViewItem &option)
{
    auto emCol = static_cast<MusicItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicItemDelegate::Number:
        return option.rect.marginsRemoved(QMargins(MusicItemLeftMargin, 0, 0, 0));
    case MusicItemDelegate::Title:
    case MusicItemDelegate::Artist:
    case MusicItemDelegate::Album:
        return option.rect.marginsRemoved(QMargins(0, 0, 0, 0));
    case MusicItemDelegate::Length:
        return option.rect.marginsRemoved(QMargins(0, 0, MusicItemRightMargin, 0));
    case MusicItemDelegate::ColumnButt:
        break;
    }
    return option.rect.marginsRemoved(QMargins(0, 0, 0, 0));
}

static inline QString numberString(int row, const QStyleOptionViewItem &option)
{
    auto listview = qobject_cast<const MusicListView *>(option.widget);
    auto itemCount = listview->model()->rowCount();
    auto itemCountString = QString("%1").arg(itemCount);
    return QString("%1").arg(int(row), itemCountString.length(), 10, QChar('0'));
}

static inline int pixel2point(int pixel)
{
    return pixel * 96 / 72;
}


void MusicItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_D(const MusicItemDelegate);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);

    auto background = (index.row() % 2) == 0 ? d->background() : d->alternateBackground();
    auto textColor = d->foreground(index.column(), option);

    if (option.state & QStyle::State_Selected) {
        background = d->highlightedBackground();
    }


    painter->fillRect(option.rect, background);

    auto flag = alignmentFlag(index.column());
    auto rect = colRect(index.column(), option);

    painter->setPen(textColor);

    switch (index.column()) {
    case Number: {
        auto *w = const_cast<QWidget *>(option.widget);
        bool hideAnimation = false;
        if (!d->playingIndex.isValid()) {
            hideAnimation = true;
        } else {
            auto listview = qobject_cast<MusicListView *>(w);
            auto viewRect = QRect(QPoint(0, 0), listview->viewport()->size());
            if (!viewRect.intersects(listview->visualRect(d->playingIndex))) {
                hideAnimation = true;
            }
        }
        if (hideAnimation) {
            d->playingAnimation->hide();
        }

        if (d->playingIndex == index && !hideAnimation) {
            d->playingAnimation->setParent(w);
            d->playingAnimation->raise();
            d->playingAnimation->play();
            d->playingAnimation->show();
            auto center = option.rect.center();
            auto aniSize = d->playingAnimation->size();
            d->playingAnimation->move(center.x() - aniSize.width() / 2, center.y() - aniSize.height() / 2);
        } else {
            auto num = numberString(index.row() + 1, option);
            painter->drawText(rect, flag, num);
        }


        break;
    }
    case Title:
    case Artist:
    case Album:
    case Length:
        painter->drawText(rect, flag, index.data().toString());
        break;
    default:
        break;
    }
//    auto lineRect = QRect(option.rect.bottomLeft(), option.rect.bottomRight());
//    painter->fillRect(lineRect, QColor(0x79, 0x79, 0x79, 26));
    painter->restore();
}

inline int headerPointWidth(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QFont measuringFont(option.font);
    QFontMetrics fm(measuringFont);
    auto headerWith = fm.width(QString("%1").arg(index.row()));
    return pixel2point(headerWith) + MusicItemLeftMargin + MusicItemNumberMargin;
}

inline int tailPointWidth(const QStyleOptionViewItem &option)
{
    QFont measuringFont(option.font);
    QFontMetrics fm(measuringFont);
    return pixel2point(fm.width("00:00")) + MusicItemRightMargin;
}

QSize MusicItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    auto baseSize = QStyledItemDelegate::sizeHint(option, index);
    auto headerWidth = headerPointWidth(option, index);
    auto tialWidth = tailPointWidth(option);
    auto w = option.widget->width() - headerWidth - tialWidth;
    Q_ASSERT(w > 0);
    switch (index.column()) {
    case 0:
        return  QSize(headerWidth, baseSize.height());
    case 1:
        return  QSize(w / 2, baseSize.height());
    case 2:
    case 3:
        return  QSize(w / 4, baseSize.height());
    case 4:
        return  QSize(tialWidth, baseSize.height());
    }

    return baseSize;
}

QWidget *MusicItemDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const

{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void MusicItemDelegate::setEditorData(QWidget *editor,
                                      const QModelIndex &index) const
{

    QStyledItemDelegate::setEditorData(editor, index);

}

void MusicItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

MusicItemDelegate::MusicItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent), d_ptr(new MusicItemDelegatePrivate(this))
{

}

MusicItemDelegate::~MusicItemDelegate()
{
}

void MusicItemDelegate::setPlayingIndex(const QModelIndex &index)
{
    Q_D(MusicItemDelegate);
    d->playingIndex = index;
}

void MusicItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->state = option->state & ~QStyle::State_HasFocus;
}
