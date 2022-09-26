// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QHBoxLayout>

#include <DFrame>

#include "songlistviewdialog.h"
#include "songlistview.h"
#include "ac-desktop-define.h"

DWIDGET_USE_NAMESPACE
SongListViewDialog::SongListViewDialog(QWidget *parent) : DDialog(parent)
{
    this->setObjectName(AC_tablet_songListViewDlg);
    m_songListView = new SongListView(this);
    m_songListView->setObjectName(AC_tablet_songListView);
    DFrame  *d = new DFrame(this);
    QVBoxLayout *hb = new QVBoxLayout(d);
    hb->addWidget(m_songListView, Qt::AlignCenter);
    hb->setSpacing(0);
    d->setLayout(hb);
    d->setFixedSize(480, 430);
    this->addContent(d, Qt::AlignCenter);
    setFixedSize(500, 500);
}
