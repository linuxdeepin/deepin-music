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

#include "musicbaseandsonglistmodel.h"

#include <QDebug>
#include <QMimeData>
#include <DStandardItem>

DWIDGET_USE_NAMESPACE

MusicBaseAndSonglistModel::MusicBaseAndSonglistModel(QObject *parent) :
    QStandardItemModel(parent)
{

}

MusicBaseAndSonglistModel::~MusicBaseAndSonglistModel()
{

}

bool MusicBaseAndSonglistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QStandardItemModel::setData(index, value, role);
}

//Qt::DropActions MusicBaseAndSonglistModel::supportedDropActions() const
//{
//    return Qt::CopyAction;
//}

QMimeData *MusicBaseAndSonglistModel::mimeData(const QModelIndexList &indexes) const
{
    auto curMimeData = QStandardItemModel::mimeData(indexes);
    if (curMimeData != nullptr && indexes.count() == 1) {
        auto curSelectedItem = static_cast<DStandardItem *>(itemFromIndex(indexes.first()));
        if (curSelectedItem->row() != 0 || curSelectedItem->data(Qt::UserRole + 12).toString() != "CdaRole") {
            curMimeData->setData("CdaRole", QByteArray::number(1));
            curMimeData->setData("ROW", QByteArray::number(curSelectedItem->row()));
        }
    }
    return curMimeData;
}

Qt::ItemFlags MusicBaseAndSonglistModel::flags(const QModelIndex &index) const
{
    return index.isValid() ? Qt::ItemIsDropEnabled | QStandardItemModel::flags(index) : QStandardItemModel::flags(index);
}

