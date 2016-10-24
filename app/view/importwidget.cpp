/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "importwidget.h"

#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QDebug>

#include <dthememanager.h>
#include <dbasebutton.h>

DWIDGET_USE_NAMESPACE

#include "../musicapp.h"

const QString linkTemplate = "<a href='%1' style='text-decoration: none;'>%2</a>";

ImportWidget::ImportWidget(QWidget *parent) : QFrame(parent)
{
    setObjectName("ImportWidget");
    auto layout = new QVBoxLayout(this);

    auto logo = new QLabel;
    logo->setFixedSize(128, 128);
    logo->setObjectName("ImportViewLogo");

    auto importButton = new Dtk::Widget::DBaseButton;
    importButton->setObjectName("ImportViewImportButton");
    importButton->setFixedSize(130, 36);
    importButton->setText(tr("Import"));

    auto text = new QLabel;
    text->setObjectName("ImportViewText");
    text->setFixedHeight(24);
    text->setOpenExternalLinks(false);
    QString linkText = QString(linkTemplate).arg(tr("Scan")).arg(tr("Scan"));
    text->setText(QString(tr("You can %1 or drop music here")).arg(linkText));


    layout->addStretch();
    layout->addWidget(logo, 0, Qt::AlignCenter);
    layout->addSpacing(40);
    layout->addWidget(importButton, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(text, 0, Qt::AlignCenter);
    layout->addStretch();

    D_THEME_INIT_WIDGET(ImportWidget);

    connect(importButton, &QPushButton::clicked,
    this, [ = ] {
        emit this->importFiles();
    });

    connect(text, &QLabel::linkActivated,
    this, [ = ](const QString & /*link*/) {
        emit this->importMusicDirectory();
    });
}
