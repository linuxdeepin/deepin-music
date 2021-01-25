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

#include <QScopedPointer>
#include <QStyledItemDelegate>


class SingerDataDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit SingerDataDelegate(QWidget *parent = Q_NULLPTR);
    ~SingerDataDelegate() override;

    virtual void initStyleOption(QStyleOptionViewItem *option,
                                 const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

//    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
//                          const QModelIndex &index) const override;

//    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

//    void setModelData(QWidget *editor, QAbstractItemModel *model,
//                      const QModelIndex &index) const override;

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
private:
    void drawIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawListMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void mouseClicked(const QStyleOptionViewItem &option, const QModelIndex &index, const QPointF pressPos);
    void mouseDoubleClicked(const QStyleOptionViewItem &option, const QModelIndex &index);
private:
    QPixmap hoverPlayImg ;
    QPixmap hoverSuspendImg ;
    QPixmap shadowImg ;
};

