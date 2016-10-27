/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "lyricview.h"

#include <QDebug>
#include <QLabel>
#include <QFile>
#include <QScrollArea>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPainter>
#include <QResizeEvent>
#include <QPaintEvent>

#include <dthememanager.h>
DWIDGET_USE_NAMESPACE

#include "widget/cover.h"

const QString aaa =
    "You're the one that i want' the one that i need"
    "你是我想要的那个唯一，也是我需要的那个唯一\n"
    "The one that i gotta have just to succeed"
    "是唯一我想要成功的唯一\n"
    "When i first saw you' i knew it was real"
    "当我第一次看到你的时候.我知道你是我真正的唯一\n"
    "I'm sorry about the pain i made you feel"
    "真的对不起我让你陷入了痛苦的境地\n"
    "That wasn't me; let me show you the way"
    "那不是我！那不是真正的我！\n"
    "I looked for the sun' but it's raining today"
    "就像我想要太阳，天空却偏偏下起了雨一样的无奈\n"
    "You're the one that i want' the one that i need"
    "你是我想要的那个唯一，也是我需要的那个唯一\n"
    "The one that i gotta have just to succeed"
    "是唯一我想要成功的唯一\n"
    "When i first saw you' i knew it was real"
    "当我第一次看到你的时候.我知道你是我真正的唯一\n"
    "I'm sorry about the pain i made you feel"
    "真的对不起我让你陷入了痛苦的境地\n"
    "That wasn't me; let me show you the way"
    "那不是我！那不是真正的我！\n"
    "I looked for the sun' but it's raining today"
    "就像我想要太阳，天空却偏一样的无奈\n"
    "You're the one that i want' the one that i need"
    "你是我想要的那个唯一，也是我需要的那个唯一\n"
    "The one that i gotta have just to succeed"
    "是唯一我想要成功的唯一\n"
    "When i first saw you' i knew it was real"
    "当我第一次看到你的时候.我知道你是我真正的唯一\n"
    "I'm sorry about the pain i made you feel"
    "真的对不起我让你陷入了痛苦的境地\n"
    "That wasn't me; let me show you the way"
    "那不是我！那不是真正的我！\n"
    "I looked for the sun' but it's raining today"
    "就像我想要太阳，天空却偏偏下起了雨一样的无奈\n";


LyricView::LyricView(QWidget *parent) : QFrame(parent)
{
    setObjectName("LyricView");
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);

    m_cover = new Cover;
    m_cover->setFixedSize(200, 200);
    m_cover->setObjectName("LyricCoveraa");
    m_cover->setBackgroundUrl(":/image/cover_max.png");

    m_scroll = new QScrollArea;
    m_scroll->setObjectName("LyricTextScroll");

    m_lyric = new QLabel;
    m_lyric->setObjectName("LyricText");
    setLyricLines(aaa.split("\n"));
    m_lyric->setWordWrap(true);

    m_scroll->setWidget(m_lyric);
    m_scroll->setAlignment(Qt::AlignCenter);
    m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto btBack = new QPushButton;
    btBack->setObjectName("LyricBack");

    QSizePolicy spCover(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spCover.setHorizontalStretch(80);
    m_cover->setSizePolicy(spCover);
    layout->addWidget(m_cover, 0, Qt::AlignCenter);

    QSizePolicy spText(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spText.setHorizontalStretch(20);
    m_scroll->setSizePolicy(spText);

    layout->addWidget(m_scroll, 0, Qt::AlignCenter);

    QSizePolicy spBack(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spBack.setHorizontalStretch(20);
    btBack->setSizePolicy(spBack);
    btBack->setFixedSize(27, 23);

    layout->addWidget(btBack, 0, Qt::AlignRight | Qt::AlignTop);


    D_THEME_INIT_WIDGET(LyricView);
}

void LyricView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_scroll->setFixedSize(event->size().width() * 45 / 100, event->size().height() * 90 / 100);
    m_lyric->setFixedWidth(event->size().width() * 35 / 100);
}

void LyricView::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    QBrush brush(QColor(255, 255, 255, 25));
    auto hcenter = m_scroll->y() + m_scroll->height() / 2;
    auto xstart = m_scroll->x();
    auto xend = m_scroll->x() + m_scroll->width();

    QPainterPath path;
    path.moveTo(xstart, hcenter - 4);
    path.lineTo(xstart, hcenter + 5);
    path.lineTo(xstart + 9, hcenter);
    path.lineTo(xstart, hcenter - 4);

    path.moveTo(xstart + 20, hcenter);
    path.lineTo(xend - 20, hcenter);
    path.lineTo(xend - 20, hcenter + 1);
    path.lineTo(xstart + 20, hcenter + 1);
    path.lineTo(xstart + 20, hcenter);

    path.moveTo(xend, hcenter - 4);
    path.lineTo(xend, hcenter + 5);
    path.lineTo(xend - 9, hcenter);
    path.lineTo(xend, hcenter - 4);

    painter.fillPath(path, brush);
}

void LyricView::setLyricLines(const QStringList &lines)
{
    QString lyric;
    for (auto line : lines) {
        if (line.isEmpty()) {
            lyric.append("<br />");
        } else {
        }
        lyric.append(QString("<p style='line-height:180%'>%1</p>").arg(line));
    }
    m_lyric->setText(lyric);
}

void LyricView::onLyricChanged(const MusicMeta &info, const QString &lyricPath)
{
    QFile lyricFile(lyricPath);
    qDebug() << lyricPath;
    if (!lyricFile.open(QIODevice::ReadOnly)) {
        this->setLyricLines(QStringList());
        return;
    }
    auto lyric = QString::fromUtf8(lyricFile.readAll());
    this->setLyricLines(lyric.split("\n"));
}

void LyricView::onCoverChanged(const MusicMeta &info, const QString &coverPath)
{
    m_cover->setBackgroundUrl("");
    m_cover->setBackgroundUrl(coverPath);
    m_cover->repaint();
}
