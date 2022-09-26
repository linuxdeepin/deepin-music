// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    SongListViewModel *m_listViewModel = nullptr;
    QWidget           *m_songListViewDialog = nullptr;
};
#endif // SONGLISTVIEW_H
