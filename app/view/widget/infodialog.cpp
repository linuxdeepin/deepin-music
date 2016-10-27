/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "infodialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>

#include <dthememanager.h>
#include <dwindowclosebutton.h>

#include "../../model/musiclistmodel.h"

InfoDialog::InfoDialog(const MusicMeta &info, QWidget *parent) : DAbstractDialog(parent)
{
    setFixedSize(320, 480);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(5);

    auto closeBt = new DWindowCloseButton;
    closeBt->setObjectName("InfoClose");
    closeBt->setFixedSize(24, 24);
    closeBt->setAttribute(Qt::WA_NoMousePropagation);

    auto cover = new QLabel;
    cover->setContentsMargins(0, 0, 0, 0);
    cover->setObjectName("InfoCover");
    cover->setFixedSize(140, 140);
    cover->setPixmap(QPixmap(":/image/cover_max.png").scaled(140, 140));

    auto title = new QLabel(info.title);
    title->setObjectName("InfoTitle");
    title->setFixedHeight(18);

    auto split = new QLabel();
    split->setObjectName("InfoSplit");
    split->setFixedSize(300, 2);

    QStringList infoKeys;
    infoKeys << tr("Title:") << tr("Artist:") << tr("Album:")
             << tr("File type:") << tr("Size:") << tr("Length:")
             << tr("Directory:");

    QStringList infoValues;
    infoValues << info.title << info.artist << info.album
               << info.filetype << sizeString(info.size) << lengthString(info.length)
               << info.localpath;

    auto infogridLayout = new QGridLayout;
    infogridLayout->setMargin(10);
    infogridLayout->setHorizontalSpacing(10);
    infogridLayout->setVerticalSpacing(6);

    for (int i = 0; i < infoKeys.length(); ++i) {
        auto infoKey = new QLabel(infoKeys.value(i));
        infoKey->setObjectName("InfoKey");
        infoKey->setFixedHeight(18);

        auto infoValue = new QLabel(infoValues.value(i));
        infoValue->setObjectName("InfoValue");
//        infoValue->setMaximumWidth(250);

        infogridLayout->addWidget(infoKey, i, 0, Qt::AlignRight);
        infogridLayout->addWidget(infoValue, i, 1, Qt::AlignLeft);

        if (i == infoKeys.length() - 1) {
            infoValue->setWordWrap(true);
        }
    }

    layout->addWidget(closeBt, 0, Qt::AlignTop | Qt::AlignRight);
    layout->addSpacing(43);
    layout->addWidget(cover, 0, Qt::AlignCenter);
    layout->addSpacing(13);
    layout->addWidget(title, 0, Qt::AlignCenter);
    layout->addSpacing(19);
    layout->addWidget(split, 0, Qt::AlignCenter);
    layout->addLayout(infogridLayout);
    layout->addStretch();

    D_THEME_INIT_WIDGET(InfoDialog);

    connect(closeBt, &DWindowCloseButton::clicked, this, &DAbstractDialog::close);
}
