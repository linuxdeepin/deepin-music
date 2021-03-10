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

#include "songlistviewdialog.h"
#include "songlistview.h"
#include <DFrame>
#include <QHBoxLayout>

#ifdef TABLET_PC
DWIDGET_USE_NAMESPACE
SongListViewDialog::SongListViewDialog(QWidget *parent) : DDialog(parent)
{
    m_songListView = new SongListView(this);
    DFrame  *d = new DFrame;
    QVBoxLayout *hb = new QVBoxLayout(d);
    hb->addWidget(m_songListView, Qt::AlignCenter);
    hb->setSpacing(0);
    d->setLayout(hb);
    d->setFixedSize(480, 430);
    this->addContent(d, Qt::AlignCenter);
    setFixedSize(500, 500);
}
#endif
