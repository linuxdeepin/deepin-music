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

#ifndef SONGLISTVIEW_H
#define SONGLISTVIEW_H

#include <DListView>

DWIDGET_USE_NAMESPACE

class SongListViewModel;
class SongListView : public DListView
{
    Q_OBJECT

    // Item高度
    static constexpr int ItemHeight = 48;
    // Icon边长
    static constexpr int ItemIconSide = 24;
public:
    explicit SongListView(QWidget *parent = nullptr);
    //void clearData();
    void addSongListItem(const QString &hash, const QString &name, const QIcon &icon);
    void setThemeType(int type);
signals:
    void signalItemTriggered(const QString &hash, const QString &name);
public slots:
    void slotItemChanged(const QModelIndex &model);
private:
    SongListViewModel *m_listViewModel;
};
#endif // SONGLISTVIEW_H
