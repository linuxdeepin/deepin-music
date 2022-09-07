// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SONGLISTVIEWDIALOG_H
#define SONGLISTVIEWDIALOG_H

#include <DDialog>

DWIDGET_USE_NAMESPACE
class  SongListView;

class SongListViewDialog : public DDialog
{
    Q_OBJECT
public:
    explicit SongListViewDialog(QWidget *parent = nullptr);

signals:

public slots:
public:
    SongListView *m_songListView;
};

#endif // SONGLISTVIEWDIALOG_H
