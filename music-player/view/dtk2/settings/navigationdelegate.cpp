/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "navigationdelegate.h"

#include <QPainter>
#include <QDebug>

class NavigationDelegatePrivate
{
public:
    NavigationDelegatePrivate(NavigationDelegate *parent) : q_ptr(parent) {}

    NavigationDelegate *q_ptr;
    Q_DECLARE_PUBLIC(NavigationDelegate)
};

NavigationDelegate::NavigationDelegate(QWidget *parent) :
    QStyledItemDelegate(parent) ,
    d_ptr(new NavigationDelegatePrivate(this))
{

}

NavigationDelegate::~NavigationDelegate()
{

}

void NavigationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);

    painter->setBrush(option.palette.foreground());

    auto level = static_cast<NavLevel>(index.data(NavLevelRole).toInt());

    bool isSelected = option.state & QStyle::State_Selected;

    switch (level) {
    case Level1:
    case Level2: {
        if (isSelected) {
            auto brush =  QColor("#d5edfe");
            painter->fillRect(option.rect, brush);
            auto rect = option.rect;
            rect.setX(option.rect.x() + option.rect.width() - 3);
            brush =  QColor("#2ca7f8");
            painter->fillRect(rect, brush);
        }
    }
    case Split:
    case Level3:
        break;
    }

    switch (level) {
    case Level1: {
        auto pen = isSelected ? QColor("#2ca7f8") : QColor("#00162c");
        painter->setPen(pen);
        auto rect = option.rect.marginsRemoved(QMargins(30, 0, 0, 0));
        auto font = painter->font();
        font.setPixelSize(16);
        font.setWeight(QFont::DemiBold);
        painter->setFont(font);

        QFontMetrics fm(font);
        auto text = fm.elidedText(index.data().toString(), Qt::ElideMiddle, 150);
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text);
        break;
    }
    case Level2: {
        auto pen = isSelected ? QColor("#2ca7f8") : QColor("#303030");
        painter->setPen(pen);
        auto font = painter->font();
        font.setPixelSize(12);
        painter->setFont(font);

        QFontMetrics fm(font);
        auto text = fm.elidedText(index.data().toString(), Qt::ElideMiddle, 135);
        auto rect = option.rect.marginsRemoved(QMargins(45, 0, 0, 0));
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text);
        break;
    }
    case Split:
    case Level3:
        break;
    }

    painter->restore();
}

QSize NavigationDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    auto sh = QStyledItemDelegate::sizeHint(option, index);

    auto level = static_cast<NavLevel>(index.data(NavLevelRole).toInt());
    switch (level) {
    case Split: {
        sh.setHeight(20);
        break;
    }
    case Level1:
    case Level2:
    case Level3:
        sh.setHeight(30);
        break;
    }
    return sh;
}

QWidget *NavigationDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const

{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void NavigationDelegate::setEditorData(QWidget *editor,
                                       const QModelIndex &index) const
{

    QStyledItemDelegate::setEditorData(editor, index);

}

void NavigationDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

void NavigationDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
}
