// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

