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
#include <QDragEnterEvent>
#include <QMimeData>
#include <QGraphicsOpacityEffect>

#include <dbasebutton.h>

#include "view/helper/thememanager.h"

DWIDGET_USE_NAMESPACE


const QString linkTemplate = "<a href='%1' style='text-decoration: none; color: #0082FA; '>%2</a>";

class ImportWidgetPrivate
{
public:
    ImportWidgetPrivate(ImportWidget *parent) : q_ptr(parent) {}

    QLabel                  *text = nullptr;
    QPushButton             *importButton = nullptr;

    ImportWidget *q_ptr;
    Q_DECLARE_PUBLIC(ImportWidget)
};

ImportWidget::ImportWidget(QWidget *parent) : QFrame(parent), d_ptr(new ImportWidgetPrivate(this))
{
    Q_D(ImportWidget);
    setObjectName("ImportWidget");

    ThemeManager::instance()->regisetrWidget(this);

    setAcceptDrops(true);

    auto layout = new QVBoxLayout(this);

    auto logo = new QLabel;
    logo->setFixedSize(128, 128);
    logo->setObjectName("ImportViewLogo");

//    auto btFrme = new QFrame;
//    auto btFrmeLayout = new QVBoxLayout(btFrme);
//    btFrme->setFixedSize(150, 50);
    d->importButton = new QPushButton;
    d->importButton->setObjectName("ImportViewImportButton");
    d->importButton->setFixedSize(142, 42);
//    btFrmeLayout->addWidget(d->importButton, 0, Qt::AlignCenter);
    d->importButton->setText(tr("Add folder"));

    d->text = new QLabel;
    d->text->setObjectName("ImportViewText");
    d->text->setFixedHeight(24);
    d->text->setOpenExternalLinks(false);
    QString linkText = QString(linkTemplate).arg(tr("Scan")).arg(tr("Scan"));
    d->text->setText(QString(tr("%1 music directory or drag & drop music file to add music")).arg(linkText));

    layout->addStretch();
    layout->addWidget(logo, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(d->importButton, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(d->text, 0, Qt::AlignCenter);
    layout->addStretch();

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
    d->importButton->hide();
    d->text->setText(tr("Loading music, please wait..."));
}

void ImportWidget::showImportHint()
{
    Q_D(ImportWidget);
    d->importButton->setDisabled(false);
    d->importButton->show();
    QString linkText = QString(linkTemplate).arg(tr("Scan")).arg(tr("Scan"));
    d->text->setText(QString(tr("%1 music directory or drag & drop music file to add music")).arg(linkText));
}

void ImportWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return;
    }

    QFrame::dragEnterEvent(event);
}

void ImportWidget::dragMoveEvent(QDragMoveEvent *)
{

}

void ImportWidget::dragLeaveEvent(QDragLeaveEvent *)
{

}

void ImportWidget::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("text/uri-list")) {
        return;
    }

    auto urls = event->mimeData()->urls();
    QStringList localpaths;
    for (auto &url : urls) {
        localpaths << url.toLocalFile();
    }

    if (!localpaths.isEmpty()) {
        emit importSelectFiles(localpaths);
    }
}
