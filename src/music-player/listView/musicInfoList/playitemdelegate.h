// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

public:
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

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

