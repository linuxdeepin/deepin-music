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

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class PlayItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum MusicColumn {
        Number  = 0,
        Title,
        Artist,
        Album,
        Length,

        ColumnButt,
    };
    Q_ENUM(MusicColumn)

    enum MusicItemDataRole {
        PlayStatusRole = Qt::UserRole + 100,
    };
    Q_ENUM(MusicItemDataRole)

    enum PlayStatus {
        Playing = 0x0098,
        Strop,
    };
    Q_ENUM(PlayStatus)
public:
    explicit PlayItemDelegate(QWidget *parent = Q_NULLPTR);
    ~PlayItemDelegate();
private:
//    QColor foreground(int col, const QStyleOptionViewItem &option) const;
public:
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
//    void performancePaint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
//    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
//    void setEditorData(QWidget *editor, const QModelIndex &index) const;
//    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
private:
    // 平板模式
    void drawTabletIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawTabletListMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    // pc模式
    void drawIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawListMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
public:
    QColor m_textColor;
    QColor m_numberColor;
    QColor m_highlightText;
    QColor m_background;
    QColor m_alternateBackground;
    QColor m_highlightedBackground;
    QPixmap m_shadowImg;
    QPixmap m_selectedPix;
    QPixmap m_unselectedPix;
};

