// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QScopedPointer>
#include <QStyledItemDelegate>


class AlbumDataDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit AlbumDataDelegate(QWidget *parent = Q_NULLPTR);
    ~AlbumDataDelegate();

    void initStyleOption(QStyleOptionViewItem *option,
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

signals:
    void hoverPress(const QModelIndex &index);

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
private:
    void drawIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    // 平板
    void drawTabletIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void touchClicked(const QStyleOptionViewItem &option, const QModelIndex &index, const QPointF pressPos);
    void touchDoubleClicked(const QStyleOptionViewItem &option, const QModelIndex &index);
    // pc
    void drawListMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void mouseClicked(const QStyleOptionViewItem &option, const QModelIndex &index, const QPointF pressPos);
    void mouseDoubleClicked(const QStyleOptionViewItem &option, const QModelIndex &index);
private:
    QPixmap hoverPlayImg ;
    QPixmap hoverSuspendImg ;
    QPixmap shadowImg ;
};

