/*
 * Copyright (C) 2020 chengdu Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <zhaoyongming@uniontech.com>
 *
 * Maintainer: Iceyer <zhaoyongming@uniontech.com>
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
#include <QScopedPointer>
#include <QStyledItemDelegate>
#include <DListView>

class MusicSearchListDelegatePrivate;
class MusicSearchListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    MusicSearchListDelegate(QWidget *parent = Q_NULLPTR);
    ~MusicSearchListDelegate();

protected:
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
private:
    QString getElidedText(QFont font, QString str, int MaxWidth);
    QScopedPointer<MusicSearchListDelegatePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusicSearchListDelegate)
};
