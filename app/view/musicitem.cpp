/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musicitem.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QStyle>
#include <QProcess>

#include <dthememanager.h>
#include <DMenu>
DWIDGET_USE_NAMESPACE

#include "../model/musiclistmodel.h"
#include "../core/playlist.h"
#include "../musicapp.h"
#include "widget/infodialog.h"

MusicItem::MusicItem(int num, const MusicInfo &info, QWidget *parent)
    : QWidget(parent), m_info(info)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 5, 20, 5);

    number = new QLabel;
    number->setObjectName("MusicNumber");
    number->setText(QString("%1").arg(num));

    auto titel = new QLabel;
    titel->setObjectName("MusicTitle");
    titel->setText(info.title);

    auto  artist = new QLabel;
    artist->setObjectName("MusciArtist");
    artist->setText(info.artist);

    auto album = new QLabel;
    album->setObjectName("MusicAlbum");
    album->setText(info.album);

    auto length = new QLabel;
    length->setObjectName("MusicLength");
    length->setText(lengthString(info.length));

    QSizePolicy spNumber(QSizePolicy::Fixed, QSizePolicy::Preferred);
    number->setSizePolicy(spNumber);
    number->setFixedWidth(20);
    layout->addWidget(number, 0, Qt::AlignCenter);

    QSizePolicy spTitle(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spTitle.setHorizontalStretch(2);
    titel->setSizePolicy(spTitle);
    layout->addWidget(titel, 0, Qt::AlignLeft | Qt::AlignVCenter);

    QSizePolicy spArtist(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spArtist.setHorizontalStretch(1);
    artist->setSizePolicy(spArtist);
    layout->addWidget(artist, 0, Qt::AlignLeft | Qt::AlignVCenter);

    QSizePolicy spAlbum(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spAlbum.setHorizontalStretch(1);
    album->setSizePolicy(spAlbum);
    layout->addWidget(album, 0, Qt::AlignLeft | Qt::AlignVCenter);

    QSizePolicy spLength(QSizePolicy::Fixed, QSizePolicy::Preferred);
    length->setSizePolicy(spLength);
    length->setFixedWidth(40);
    layout->addWidget(length, 0, Qt::AlignCenter);

    setFixedHeight(36);
    D_THEME_INIT_WIDGET(MusicItem);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MusicItem::customContextMenuRequested,
            this, &MusicItem::showContextMenu);

    connect(this, &MusicItem::stop, this,  &MusicItem::onMusicStop);
}

void MusicItem::onMusicPlay()
{
    number->setProperty("playstatus", "active");
    this->style()->unpolish(number);
    this->style()->polish(number);
}

void MusicItem::onMusicStop()
{
    number->setProperty("playstatus", "");
    this->style()->unpolish(number);
    this->style()->polish(number);
}

#include <QUrl>
void MusicItem::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = this->mapToGlobal(pos);

    DMenu playlistMenu;

    bool hasAction = false;
    for (auto playlist : MusicApp::presenter()->allplaylist()) {
        if (playlist->id() == "All") {
            continue;
        }
        if (playlist->id() == MusicApp::presenter()->lastPlaylist()->id()) {
            continue;
        }
        if (playlist->id() == "Fav") {
            hasAction = true;
            auto act = playlistMenu.addAction(tr("My favorites"));
            act->setData("Fav");
            continue;
        }
        auto act = playlistMenu.addAction(playlist->displayName());
        act->setData(playlist->id());
        hasAction = true;
    }
    if (hasAction) {
        playlistMenu.addSeparator();
    }
    playlistMenu.addAction(tr("New playlist"))->setData("New");

    // TODO: add all list
    connect(&playlistMenu, &DMenu::triggered, this, [ = ](DAction * action) {
        qDebug() << action->data().toString();
        QString act = action->data().toString();
        qDebug() << "addToPlaylist" << act;
        emit addToPlaylist(act);
    });

    DMenu myMenu;
    myMenu.addAction(tr("Play"));
    myMenu.addAction(tr("Add to playlist"))->setMenu(&playlistMenu);
    myMenu.addSeparator();
    myMenu.addAction(tr("Display in file manager"));
    myMenu.addAction(tr("Remove from list"));
    myMenu.addAction(tr("Delete"));
    myMenu.addSeparator();
    myMenu.addAction(tr("Song info"));

    connect(&myMenu, &DMenu::triggered, this, [ = ](DAction * action) {
        qDebug() << action;
        if (action->text() == tr("Play")) {
            emit play();
        }

        if (action->text() == tr("Display in file manager")) {
            // TODO: better
            auto dirUrl = QUrl::fromLocalFile(QFileInfo(m_info.url).absoluteDir().absolutePath());
            QFileInfo ddefilemanger("/usr/bin/dde-file-manager");
            if (ddefilemanger.exists()) {
                auto dirFile = QUrl::fromLocalFile(QFileInfo(m_info.url).absoluteFilePath());
                auto url = QString("%1?selectUrl=%2").arg(dirUrl.toString()).arg(dirFile.toString());
                QProcess::startDetached("dde-file-manager" , QStringList() << url);
            } else {
                QProcess::startDetached("gvfs-open" , QStringList() << dirUrl.toString());
            }
        }

        if (action->text() == tr("Remove from list")) {
            emit this->remove();
        }

        if (action->text() == tr("Song info")) {
            InfoDialog dlg(m_info);
            dlg.exec();
        }
    });

    myMenu.exec(globalPos);
}
