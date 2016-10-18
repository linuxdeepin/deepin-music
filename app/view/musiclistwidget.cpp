/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musiclistwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QStringListModel>

#include <QMediaPlayer>

#include <dthememanager.h>
#include <dcombobox.h>
DWIDGET_USE_NAMESPACE

#include "musiclistview.h"
#include "musicitem.h"
#include "../model/musiclistmodel.h"

MusicListWidget::MusicListWidget(QWidget *parent) : QFrame(parent)
{
    setObjectName("MusicListWidget");

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QWidget *actionBar = new QFrame;
    actionBar->setFixedHeight(40);
    actionBar->setObjectName("MusicListActionBar");

    auto actionBarLayout = new QHBoxLayout(actionBar);
    actionBarLayout->setContentsMargins(20, 0, 20, 0);
    actionBarLayout->setSpacing(0);

    auto btPlayAll = new QPushButton;
    btPlayAll->setObjectName("MusicListPlayAll");
    btPlayAll->setText(tr("Play All"));

    auto cbSort = new DComboBox;
    cbSort->setObjectName("MusicListSort");
    cbSort->addItem(tr("By Title"));
    cbSort->addItem(tr("By Add Time"));

    actionBarLayout->addWidget(btPlayAll, 0, Qt::AlignCenter);
    actionBarLayout->addStretch();
    actionBarLayout->addWidget(cbSort, 0, Qt::AlignCenter);
    m_musiclist = new MusicListView;

    layout->addWidget(actionBar, 0, Qt::AlignTop);
    layout->addWidget(m_musiclist, 0, Qt::AlignTop);

    m_model = new QStringListModel;
    m_musiclist->setModel(m_model);

    D_THEME_INIT_WIDGET(MusicListWidget);

    connect(m_musiclist, &MusicListView::doubleClicked, this, [ = ](const QModelIndex & index) {
        auto musicItem = qobject_cast<MusicItem *>(m_musiclist->indexWidget(index));
        qDebug() << musicItem << musicItem->info().url;
        if (musicItem) {
            emit musicClicked(musicItem->info());
        }
    });
}

void MusicListWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << event;
    QWidget::resizeEvent(event);
    m_musiclist->setFixedHeight(event->size().height() - 40);
}

void MusicListWidget::onMusicAdded(const MusicInfo &info)
{
    m_model->insertRows(m_model->rowCount(), 1);
    auto index = m_model->index(m_model->rowCount() - 1);
    m_musiclist->setIndexWidget(index,
                                new MusicItem(m_model->rowCount(), info));
}

void MusicListWidget::onMusicListChanged(const MusicListInfo &musiclist)
{
    for (auto &info : musiclist.list) {
        m_model->insertRows(m_model->rowCount(), 1);
        m_musiclist->setIndexWidget(m_model->index(m_model->rowCount() - 1),
                                    new MusicItem(m_model->rowCount(), info));
    }
}
