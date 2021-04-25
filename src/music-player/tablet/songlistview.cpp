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

#include "songlistview.h"
#include "songlistviewmodel.h"

#include <DStandardItem>
#include <DApplicationHelper>
#include <DDialog>
#include <DFrame>

#include "commonservice.h"

DWIDGET_USE_NAMESPACE

SongListView::SongListView(QWidget *parent) : DListView(parent)
{
    setEditTriggers(NoEditTriggers);

    DStyledItemDelegate *m_delegate = new DStyledItemDelegate(this);
    auto delegateMargins = m_delegate->margins();
    delegateMargins.setLeft(10);
    m_delegate->setMargins(delegateMargins);
    m_delegate->setItemSize(QSize(460, 48));
    m_delegate->setItemSpacing(0);
    setItemDelegate(m_delegate);

    setViewportMargins(2, 0, 0, 0);

    auto font = this->font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    setFont(font);

    setFrameShape(QFrame::NoFrame);

    m_listViewModel = new SongListViewModel(this);
    this->setModel(m_listViewModel);
    setIconSize(QSize(ItemIconSide, ItemIconSide));
    setItemSize(QSize(ItemHeight, ItemHeight));
    setFixedSize(460, 420);

    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSelectionMode(QListView::SingleSelection);

    setAutoFillBackground(true); //控件本身填充背景色

    setAlternatingRowColors(true);
    connect(this, qNonConstOverload<const QModelIndex &>(&SongListView::currentChanged), this, &SongListView::slotItemChanged);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &SongListView::setThemeType);
}

//void SongListView::clearData()
//{
//    m_listViewModel->clear();
//}

void SongListView::addSongListItem(const QString &hash, const QString &name, const QIcon &icon)
{
    DStandardItem *item = new DStandardItem(icon, name);
    item->setData(hash, Qt::UserRole);
    if (DGuiApplicationHelper::instance()->themeType() == 1) {
        item->setForeground(QColor("#414D68"));
    } else {
        item->setForeground(QColor("#C0C6D4"));
    }
    m_listViewModel->appendRow(item);
}

void SongListView::slotItemChanged(const QModelIndex &model)
{
    Q_UNUSED(model)
    qDebug() << "row:" << model.row() << "this->currentIndex().row():" << this->currentIndex().row();
    QModelIndex index = this->currentIndex();
    if (index.row() < 0) {
        return;
    }

    DStandardItem *standardItem = dynamic_cast<DStandardItem *>(m_listViewModel->itemFromIndex(index));
    // 发送点击信号，传送hash和name值
    signalItemTriggered(standardItem->data(Qt::UserRole).toString(), standardItem->text());
    // 点击item关闭窗口
    Q_ASSERT(this->parent());
    QDialog *parentdialog = dynamic_cast<QDialog *>(this->parent()->parent());
    if (parentdialog) {
        parentdialog->close();
    }
    if (CommonService::getInstance()->isTabletEnvironment()) {
        emit CommonService::getInstance()->setSelectModel(CommonService::SingleSelect);
    }
}

void SongListView::setThemeType(int type)
{
    for (int i = 0; i < m_listViewModel->rowCount(); i++) {
        QModelIndex curIndex = m_listViewModel->index(i, 0);
        DStandardItem *standardItem = dynamic_cast<DStandardItem *>(m_listViewModel->itemFromIndex(curIndex));
        if (DGuiApplicationHelper::instance()->themeType() == 1) {
            standardItem->setForeground(QColor("#414D68"));
        } else {
            standardItem->setForeground(QColor("#C0C6D4"));
        }
    }

    if (type == 1) { //white
        DPalette pl = this->palette();
        QColor c(255, 255, 255); //255,255,255,0.5
        c.setAlphaF(0.5);
        pl.setColor(QPalette::Background, c);
        this->setPalette(pl);
    } else {
        DPalette pl = this->palette();
        QColor c(0, 0, 0); //0,0,0,0.5
        c.setAlphaF(0.5);
        pl.setColor(QPalette::Background, c);
        this->setPalette(pl);
    }
}
