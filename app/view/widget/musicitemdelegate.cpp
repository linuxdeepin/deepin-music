/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musicitemdelegate.h"

#include <QPainter>

#include "../../model/musiclistmodel.h"


QColor foregroundColor(int col, const QStyleOptionViewItem &option)
{
    auto headerColor = QColor(0x79, 0x79, 0x79);

    auto emCol = static_cast<MusicItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicItemDelegate::Number:
    case MusicItemDelegate::Artist:
    case MusicItemDelegate::Album:
    case MusicItemDelegate::Length:
        if (option.state & QStyle::State_Selected) {
            return option.palette.highlightedText().color();
        } else {
            return headerColor;
        }
    case MusicItemDelegate::Title:
        if (option.state & QStyle::State_Selected) {
            return option.palette.highlightedText().color();
        } else {
            return option.palette.foreground().color();
        }
    case MusicItemDelegate::ColumnButt:
        break;
    }
    return option.palette.foreground().color();
}

QFlags<Qt::AlignmentFlag> alignmentFlag(int col)
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

QRect colRect(int col, const QStyleOptionViewItem &option)
{
    auto emCol = static_cast<MusicItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicItemDelegate::Number:
        return option.rect.marginsRemoved(QMargins(20, 0, 0, 0));
    case MusicItemDelegate::Title:
    case MusicItemDelegate::Artist:
    case MusicItemDelegate::Album:
        return option.rect.marginsRemoved(QMargins(0, 0, 0, 0));
    case MusicItemDelegate::Length:
        return option.rect.marginsRemoved(QMargins(0, 0, 20, 0));
    case MusicItemDelegate::ColumnButt:
        break;
    }
    return option.rect.marginsRemoved(QMargins(0, 0, 0, 0));
}

void MusicItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);


    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->fillRect(option.rect, option.palette.background());
    }

    //headerColor = qvariant_cast<QColor>(option.widget->property("headerColor"));
    auto textColor = foregroundColor(index.column(), option);
    auto flag = alignmentFlag(index.column());
    auto rect = colRect(index.column(), option);

    painter->setPen(textColor);

    switch (index.column()) {
    case Number: {
        auto num = QString("%1").arg(index.row() + 1);
        painter->drawText(rect, flag, num);
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

    auto lineRect = QRect(option.rect.bottomLeft(), option.rect.bottomRight());
    painter->fillRect(lineRect, QColor(0x79, 0x79, 0x79, 26));
    painter->restore();
}
#include <QDebug>

int headerWidth(const QModelIndex &index)
{
    if (index.row() > 10000) {
        return 70;
    }
    if (index.row() > 1000) {
        return 60;
    }
    if (index.row() > 100) {
        return 50;
    }
    return 40;
}

QSize MusicItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    auto baseSize = QStyledItemDelegate::sizeHint(option, index);
    auto hw = headerWidth(index);
    auto w = option.widget->width() - hw - 100;
    Q_ASSERT(w > 0);
    switch (index.column()) {
    case 0:
        return  QSize(hw, baseSize.height());
    case 1:
        return  QSize(w / 2, baseSize.height());
    case 2:
    case 3:
        return  QSize(w / 4, baseSize.height());
    case 4:
        return  QSize(100, baseSize.height());
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

void MusicItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->state = option->state & ~QStyle::State_HasFocus;
}
