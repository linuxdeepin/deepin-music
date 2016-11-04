/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "lyricview.h"

#include <QLabel>
#include <QFile>
#include <QScrollArea>
#include <QPushButton>
#include <QHBoxLayout>
#include <QListView>
#include <QPainter>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QStringListModel>
#include <QAbstractItemDelegate>

#include <dthememanager.h>

#include "../core/util/lyric.h"

#include "widget/cover.h"
#include "widget/lyriclinedelegate.h"

DWIDGET_USE_NAMESPACE

const int lyricLineHeight = 40;
const QString defaultLyric = "No Lyric";

class LyricViewPrivate
{
public:
    MusicMeta           m_playingMusic;

    int                 m_emptyOffset = 0;
    int                 m_currentline = 0;
    Lyric               m_lyriclist;

    QPushButton         *m_hideLyric = nullptr;
    Cover               *m_cover    = nullptr;
    QListView           *m_lyric    = nullptr;
    QStringListModel    *m_model    = nullptr;
};

LyricView::LyricView(QWidget *parent)
    : QFrame(parent), d(new LyricViewPrivate)
{

    setObjectName("LyricView");
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);

    d->m_cover = new Cover;
    d->m_cover->setFixedSize(200, 200);
    d->m_cover->setObjectName("LyricCover");

    d->m_lyric = new QListView;
    d->m_lyric->setObjectName("LyricTextView");
    d->m_lyric->setStyleSheet(
        DThemeManager::instance()->getQssForWidget("Widget/LyricTextView")
    );

    d->m_lyric->setSelectionMode(QListView::SingleSelection);
    d->m_lyric->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->m_lyric->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->m_lyric->setItemDelegate(new LyricLineDelegate);
    d->m_lyric->setEditTriggers(QAbstractItemView::NoEditTriggers);
    d->m_lyric->setFlow(QListView::TopToBottom);

    d->m_model = new QStringListModel;
    setLyricLines("");

    d->m_lyric->setModel(d->m_model);

    d->m_hideLyric = new QPushButton;
    d->m_hideLyric->setObjectName("LyricBack");

    QSizePolicy spCover(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spCover.setHorizontalStretch(80);
    d->m_cover->setSizePolicy(spCover);
    layout->addWidget(d->m_cover, 0, Qt::AlignCenter);

    QSizePolicy spText(QSizePolicy::Preferred, QSizePolicy::Expanding);
    spText.setHorizontalStretch(20);
    spText.setVerticalStretch(100);
    d->m_lyric->setSizePolicy(spText);
    layout->addWidget(d->m_lyric, 0, Qt::AlignVCenter);

    QSizePolicy spBack(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spBack.setHorizontalStretch(20);
    d->m_hideLyric->setSizePolicy(spBack);
    d->m_hideLyric->setFixedSize(27, 23);

    layout->addWidget(d->m_hideLyric, 0, Qt::AlignRight | Qt::AlignTop);

    D_THEME_INIT_WIDGET(LyricView);

    initConnection();
}

LyricView::~LyricView()
{

}

void LyricView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    d->m_lyric->setFixedWidth(event->size().width() * 45 / 100);
    adjustLyric();
}

void LyricView::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    return;
//    QPainter painter(this);

//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setRenderHint(QPainter::HighQualityAntialiasing);

//    QBrush brush(QColor(255, 255, 255, 25));
//    auto hcenter = d->m_lyric->y() + d->m_lyric->height() / 2;
//    auto xstart = d->m_lyric->x();
//    auto xend = d->m_lyric->x() + d->m_lyric->width();

//    QPainterPath path;
//    path.moveTo(xstart, hcenter - 4);
//    path.lineTo(xstart, hcenter + 5);
//    path.lineTo(xstart + 9, hcenter);
//    path.lineTo(xstart, hcenter - 4);

//    path.moveTo(xstart + 20, hcenter);
//    path.lineTo(xend - 20, hcenter);
//    path.lineTo(xend - 20, hcenter + 1);
//    path.lineTo(xstart + 20, hcenter + 1);
//    path.lineTo(xstart + 20, hcenter);

//    path.moveTo(xend, hcenter - 4);
//    path.lineTo(xend, hcenter + 5);
//    path.lineTo(xend - 9, hcenter);
//    path.lineTo(xend, hcenter - 4);

//    painter.fillPath(path, brush);
}
#include <QDebug>
void LyricView::setLyricLines(QString str)
{
    d->m_lyriclist = parseLrc(str);

    QStringList lines;
    if (!d->m_lyriclist.hasTime) {
        lines = str.split('\n');
    } else {
        for (auto &ele : d->m_lyriclist.m_lyricElements) {
            lines << ele.content;
        }
    }

    if (lines.length() <= 0) {
        lines << defaultLyric;
    }

    d->m_currentline = 0;
    auto itemHeight = lyricLineHeight;
    auto maxHeight = this->height() * 9 / 10;
    if (lines.length() > 2) {
        d->m_emptyOffset = maxHeight / itemHeight / 2;
    } else {
        d->m_emptyOffset = 0;
    }

    QStringList lyric;

    for (int i = 0; i < d->m_emptyOffset; ++i) {
        lyric << "";
    }
    for (auto line : lines) {
        lyric << line;
    }
    for (int i = 0; i < d->m_emptyOffset; ++i) {
        lyric << "";
    }
    d->m_model->setStringList(lyric);
    d->m_lyric->setModel(d->m_model);

    QModelIndex index = d->m_model->index(
                            d->m_emptyOffset + d->m_currentline, 0, d->m_lyric->rootIndex());
    d->m_lyric->clearSelection();
    d->m_lyric->setCurrentIndex(index);
    d->m_lyric->scrollTo(index, QListView::PositionAtCenter);
    d->m_lyric->clearSelection();

    adjustLyric();
}

void LyricView::onMusicPlayed(QSharedPointer<Playlist> playlist, const MusicMeta &meta)
{
    Q_UNUSED(playlist);
    d->m_playingMusic = meta;
}

void LyricView::onProgressChanged(qint64 value, qint64 /*length*/)
{
    auto len = d->m_lyriclist.m_lyricElements.length();
    if (!d->m_lyriclist.hasTime) {
        return;
    }
    if (len <= 2) {
        return;
    }

    int i = 0;
    for (i = 0; i < len; ++i) {
        auto curEle = d->m_lyriclist.m_lyricElements.at(i);
        if (value < curEle.start) {
            break;
        }
    }
    QModelIndex index = d->m_model->index(
                            d->m_emptyOffset + i - 1, 0, d->m_lyric->rootIndex());
    d->m_lyric->clearSelection();
    d->m_lyric->setCurrentIndex(index);
    d->m_lyric->scrollTo(index, QListView::PositionAtCenter);

}

void LyricView::onLyricChanged(const MusicMeta &meta, const QString &lyricPath)
{
    if (d->m_playingMusic.hash != meta.hash) {
        return;
    }

    if (lyricPath.isEmpty()) {
        setLyricLines("");
        return;
    }

    QFile lyricFile(lyricPath);
    if (!lyricFile.open(QIODevice::ReadOnly)) {
        setLyricLines("");
        return;
    }
    auto lyricStr = QString::fromUtf8(lyricFile.readAll());
    lyricFile.close();

    setLyricLines(lyricStr);
}

void LyricView::onCoverChanged(const MusicMeta &meta, const QString &coverPath)
{
    if (d->m_playingMusic.hash != meta.hash) {
        return;
    }

    if (coverPath.isEmpty()) { return; }
    d->m_cover->setBackgroundUrl(coverPath);
    d->m_cover->repaint();
}

void LyricView::initConnection()
{
    connect(d->m_hideLyric, &QPushButton::clicked, this, &LyricView::hideLyricView);
}

void LyricView::adjustLyric()
{
    auto itemHeight = lyricLineHeight;
    auto maxHeight = this->height() * 9 / 10;
    if (d->m_model->rowCount() * itemHeight < maxHeight) {
        d->m_lyric->setFixedHeight(d->m_model->rowCount() * itemHeight);
    } else {
        d->m_lyric->setFixedHeight(maxHeight);
    }
}
