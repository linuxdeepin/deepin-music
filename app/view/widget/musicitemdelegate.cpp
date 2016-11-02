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

void MusicItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);

    painter->fillRect(option.rect, option.palette.background());
    painter->setBrush(option.palette.foreground());
    auto rect = option.rect.marginsRemoved(QMargins(10, 0, 10, 0));

    switch (index.column()) {
    case 0: {
        auto headerColor = QColor(0x79, 0x79, 0x79);
        if (option.widget) {
            headerColor = qvariant_cast<QColor>(option.widget->property("headerColor"));
        }
        if (option.state & QStyle::State_Selected) {
            painter->setPen(option.palette.highlightedText().color());
        } else {
            painter->setPen(headerColor);
        }
        auto num = QString("%1").arg(index.row()+1);
        painter->drawText(option.rect, Qt::AlignCenter, num);
    }
    break;
    case 4: {
        if (option.state & QStyle::State_Selected) {
            painter->setPen(option.palette.highlightedText().color());
        } else {
            painter->setPen(option.palette.foreground().color());
        }
        painter->drawText(option.rect, Qt::AlignCenter, index.data().toString());
    }
    break;
    default: {
        if (option.state & QStyle::State_Selected) {
            painter->setPen(option.palette.highlightedText().color());
        } else {
            painter->setPen(option.palette.foreground().color());
        }

        painter->drawText(rect, Qt::AlignVCenter, index.data().toString());
    }
    break;
    }
    painter->restore();
}

QSize MusicItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    auto sh = QStyledItemDelegate::sizeHint(option, index);

    if (0 == index.column())  {
        return QSize(40, sh.height());
    }

    return sh;
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
