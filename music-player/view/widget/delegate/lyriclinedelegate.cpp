/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "lyriclinedelegate.h"

#include <QPainter>
#include <QDebug>


class LyricLineDelegatePrivate
{
public:
    explicit LyricLineDelegatePrivate(LyricLineDelegate *parent): q_ptr(parent) {}

    QWidget *parentWidget;

    LyricLineDelegate *q_ptr;
    Q_DECLARE_PUBLIC(LyricLineDelegate)
};

void LyricLineDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);

    painter->fillRect(option.rect, option.palette.background());
    painter->setBrush(option.palette.foreground());

//    painter->setPen(Qt::blue);
//    painter->drawRect(option.rect);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.highlightedText().color());
//        painter->setPen(Qt::blue);
    } else {
        painter->setPen(option.palette.foreground().color());
    }
    painter->drawText(option.rect, Qt::AlignCenter, index.data().toString());

    painter->restore();
}

QSize LyricLineDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    Q_D(const LyricLineDelegate);

    auto sh = QStyledItemDelegate::sizeHint(option, index);
    return sh;
}

QWidget *LyricLineDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const

{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void LyricLineDelegate::setEditorData(QWidget *editor,
                                      const QModelIndex &index) const
{

    QStyledItemDelegate::setEditorData(editor, index);

}

void LyricLineDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

LyricLineDelegate::LyricLineDelegate(QWidget *parent): QStyledItemDelegate(parent), d_ptr(new LyricLineDelegatePrivate(this))
{
    Q_D(LyricLineDelegate);
    d->parentWidget = parent;
}

LyricLineDelegate::~LyricLineDelegate()
{

}

void LyricLineDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->state = option->state /*& ~QStyle::State_HasFocus*/;
}
