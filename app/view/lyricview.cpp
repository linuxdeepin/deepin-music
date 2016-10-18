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
#include <QScrollArea>
#include <QPushButton>
#include <QHBoxLayout>
#include <QResizeEvent>

#include <dthememanager.h>
DWIDGET_USE_NAMESPACE

const QString aaa =
    "You're the one that i want' the one that i need\n"
    "你是我想要的那个唯一，也是我需要的那个唯一\n\n"
    "The one that i gotta have just to succeed\n"
    "是唯一我想要成功的唯一\n\n"
    "When i first saw you' i knew it was real\n"
    "当我第一次看到你的时候.我知道你是我真正的唯一\n\n"
    "I'm sorry about the pain i made you feel\n"
    "真的对不起我让你陷入了痛苦的境地\n\n"
    "That wasn't me; let me show you the way\n"
    "那不是我！那不是真正的我！\n\n"
    "I looked for the sun' but it's raining today\n"
    "就像我想要太阳，天空却偏偏下起了雨一样的无奈\n\n"
    "You're the one that i want' the one that i need\n"
    "你是我想要的那个唯一，也是我需要的那个唯一\n\n"
    "The one that i gotta have just to succeed"
    "是唯一我想要成功的唯一\n\n"
    "When i first saw you' i knew it was real\n"
    "当我第一次看到你的时候.我知道你是我真正的唯一\n\n"
    "I'm sorry about the pain i made you feel\n"
    "真的对不起我让你陷入了痛苦的境地\n\n"
    "That wasn't me; let me show you the way\n"
    "那不是我！那不是真正的我！\n\n"
    "I looked for the sun' but it's raining today\n"
    "就像我想要太阳，天空却偏一样的无奈\n\n"
    "You're the one that i want' the one that i need\n"
    "你是我想要的那个唯一，也是我需要的那个唯一\n\n"
    "The one that i gotta have just to succeed"
    "是唯一我想要成功的唯一\n\n"
    "When i first saw you' i knew it was real\n"
    "当我第一次看到你的时候.我知道你是我真正的唯一\n\n"
    "I'm sorry about the pain i made you feel\n"
    "真的对不起我让你陷入了痛苦的境地\n\n"
    "That wasn't me; let me show you the way\n"
    "那不是我！那不是真正的我！\n\n"
    "I looked for the sun' but it's raining today\n"
    "就像我想要太阳，天空却偏偏下起了雨一样的无奈\n\n";


LyricView::LyricView(QWidget *parent) : QFrame(parent)
{
    setObjectName("LyricView");
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);

    auto cover = new QLabel;
    cover->setFixedSize(200, 200);
    cover->setObjectName("LyricCover");

    m_scroll = new QScrollArea;
    m_scroll->setObjectName("LyricTextScroll");

    m_lyric = new QLabel;
    m_lyric->setObjectName("LyricText");
    m_lyric->setText(aaa) ;
    m_lyric->setWordWrap(true);

    m_scroll->setWidget(m_lyric);
    m_scroll->setAlignment(Qt::AlignCenter);
    m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto btBack = new QPushButton;
    btBack->setObjectName("LyricBack");

    QSizePolicy spCover(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spCover.setHorizontalStretch(50);
    cover->setSizePolicy(spCover);
    layout->addWidget(cover, 0, Qt::AlignCenter);

    QSizePolicy spText(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spText.setHorizontalStretch(50);
    m_scroll->setSizePolicy(spText);
    layout->addWidget(m_scroll, 0, Qt::AlignCenter);

    QSizePolicy spBack(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spBack.setHorizontalStretch(10);
    btBack->setSizePolicy(spBack);
    btBack->setFixedSize(27, 23);
    layout->addWidget(btBack, 0, Qt::AlignRight | Qt::AlignTop);


    D_THEME_INIT_WIDGET(LyricView);
}

void LyricView::resizeEvent(QResizeEvent *event)
{
    qDebug() << event;
    QWidget::resizeEvent(event);
    m_scroll->setFixedSize(event->size().width() * 0.5, event->size().height() * 0.9);
    m_lyric->setFixedWidth(event->size().width() * 0.45);
}
