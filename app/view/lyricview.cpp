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

#include <dthememanager.h>

#include "../core/util/lyric.h"

#include "widget/cover.h"
#include "widget/lyriclinedelegate.h"

DWIDGET_USE_NAMESPACE

const QString defaultLyric = "Youth is not a time of life";

class LyricViewPrivate
{
public:
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
    d->m_cover->setObjectName("LyricCoveraa");
    d->m_cover->setBackgroundUrl(":/image/cover_max.png");

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

    d->m_model = new QStringListModel;
    setLyricLines(defaultLyric.split("\n"));

    d->m_lyric->setModel(d->m_model);

    auto btBack = new QPushButton;
    btBack->setObjectName("LyricBack");

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
    btBack->setSizePolicy(spBack);
    btBack->setFixedSize(27, 23);

    layout->addWidget(btBack, 0, Qt::AlignRight | Qt::AlignTop);

    D_THEME_INIT_WIDGET(LyricView);
}

LyricView::~LyricView()
{

}

void LyricView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    d->m_lyric->setFixedWidth(event->size().width() * 45 / 100);
    d->m_lyric->setFixedHeight(event->size().height() * 90 / 100);
    d->m_lyric->resize(event->size());
}

void LyricView::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    return;
//    QPainter painter(this);

//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setRenderHint(QPainter::HighQualityAntialiasing);

//    QBrush brush(QColor(255, 255, 255, 25));
//    auto hcenter = d->m_scroll->y() + d->m_scroll->height() / 2;
//    auto xstart = d->m_scroll->x();
//    auto xend = d->m_scroll->x() + d->m_scroll->width();

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

void LyricView::setLyricLines(const QStringList &lines)
{
    QStringList lyric;
    for (auto line : lines) {
        lyric << line;
    }
    d->m_model->setStringList(lyric);
    d->m_lyric->adjustSize();
}

void LyricView::onLyricChanged(const MusicMeta &info, const QString &lyricPath)
{
    if (lyricPath.isEmpty()) { return; }
    QFile lyricFile(lyricPath);
    if (!lyricFile.open(QIODevice::ReadOnly)) {
        this->setLyricLines(QStringList() << defaultLyric);
        return;
    }
    auto lyricStr = QString::fromUtf8(lyricFile.readAll());
    auto lyric = parseLrc(lyricStr);

    QStringList lyrics;
    for (auto &ele : lyric.m_lyricElements) {
        lyrics << ele.content;
    }
    d->m_model->setStringList(lyrics);
    d->m_lyric->adjustSize();
    lyricFile.close();
}

void LyricView::onCoverChanged(const MusicMeta &info, const QString &coverPath)
{
    d->m_cover->setBackgroundUrl(coverPath);
    d->m_cover->repaint();
}
