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
#include <QGraphicsOpacityEffect>

#include <dthememanager.h>
#include <dbasebutton.h>

DWIDGET_USE_NAMESPACE

#include "../musicapp.h"

const QString linkTemplate = "<a href='%1' style='text-decoration: none;'>%2</a>";

class ImportWidgetPrivate
{
public:
    ImportWidgetPrivate(ImportWidget *parent) : q_ptr(parent) {}

    QLabel                  *text = nullptr;
    DBaseButton             *importButton = nullptr;
    QGraphicsOpacityEffect  *hideEffect = nullptr;

    ImportWidget *q_ptr;
    Q_DECLARE_PUBLIC(ImportWidget);
};

ImportWidget::ImportWidget(QWidget *parent) : QFrame(parent), d_ptr(new ImportWidgetPrivate(this))
{
    Q_D(ImportWidget);

    setObjectName("ImportWidget");
    auto layout = new QVBoxLayout(this);

    auto logo = new QLabel;
    logo->setFixedSize(128, 128);
    logo->setObjectName("ImportViewLogo");


    d->importButton = new Dtk::Widget::DBaseButton;
    d->importButton->setObjectName("ImportViewImportButton");
    d->importButton->setFixedSize(130, 36);
    d->importButton->setText(tr("Add Music"));

    d->hideEffect = new QGraphicsOpacityEffect(d->importButton);
    d->importButton->setGraphicsEffect(d->hideEffect);
    d->hideEffect->setOpacity(1);

    d->text = new QLabel;
    d->text->setObjectName("ImportViewText");
    d->text->setFixedHeight(24);
    d->text->setOpenExternalLinks(false);
    QString linkText = QString(linkTemplate).arg(tr("Scan")).arg(tr("Scan"));
    d->text->setText(QString(tr("%1 music directory or drag & drop music file to add music")).arg(linkText));

    layout->addStretch();
    layout->addWidget(logo, 0, Qt::AlignCenter);
    layout->addSpacing(40);
    layout->addWidget(d->importButton, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(d->text, 0, Qt::AlignCenter);
    layout->addStretch();

    D_THEME_INIT_WIDGET(ImportWidget);

    connect(d->importButton, &QPushButton::clicked,
    this, [ = ] {
        emit this->importFiles();
    });

    connect(d->text, &QLabel::linkActivated,
    this, [ = ](const QString & /*link*/) {
        showWaitHint();
        emit this->scanMusicDirectory();
    });
}

ImportWidget::~ImportWidget()
{

}

void ImportWidget::showWaitHint()
{
    Q_D(ImportWidget);
    d->importButton->setDisabled(true);
    d->hideEffect->setOpacity(0);
    d->text->setText(tr("Loading music, plase wait..."));
}

void ImportWidget::showImportHint()
{
    Q_D(ImportWidget);
    d->importButton->setDisabled(false);
    d->hideEffect->setOpacity(1);
    QString linkText = QString(linkTemplate).arg(tr("Scan")).arg(tr("Scan"));
    d->text->setText(QString(tr("%1 music directory or drag & drop music file to add music")).arg(linkText));
}
