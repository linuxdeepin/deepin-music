/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
    // Item 右边边距
    static constexpr int PlayItemRightMargin = 20;
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
    // 平板icon模式
    void drawTabletIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    // 平板单击
    void touchClicked(const QStyleOptionViewItem &option, const QModelIndex &index, const QPointF pressPos);
    //平板双击
    void touchDoubleClicked(const QStyleOptionViewItem &option, const QModelIndex &index);
    // pc或者平板list模式
    void drawListMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void mouseClicked(const QStyleOptionViewItem &option, const QModelIndex &index, const QPointF pressPos);
    void mouseDoubleClicked(const QStyleOptionViewItem &option, const QModelIndex &index);
private:
    QPixmap hoverPlayImg ;
    QPixmap hoverSuspendImg ;
    QPixmap shadowImg ;
};

