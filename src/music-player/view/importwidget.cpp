/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "importwidget.h"

#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QGraphicsOpacityEffect>

#include <DFileDialog>
#include <DLabel>
#include <DPushButton>

DWIDGET_USE_NAMESPACE

const QString linkTemplate = "<a href='%1' style='text-decoration: none; color: #0082FA; '>%2</a>";

class ImportWidgetPrivate
{
public:
    ImportWidgetPrivate(ImportWidget *parent) : q_ptr(parent) {}

    DLabel                  *text = nullptr;
    DPushButton             *importButton = nullptr;

    ImportWidget *q_ptr;
    Q_DECLARE_PUBLIC(ImportWidget)
};

ImportWidget::ImportWidget(QWidget *parent) : DFrame(parent), d_ptr(new ImportWidgetPrivate(this))
{
    Q_D(ImportWidget);
    setObjectName("ImportWidget");

    setAcceptDrops(true);

    auto layout = new QVBoxLayout(this);

    auto logo = new DLabel;
    logo->setFixedSize(128, 128);
    logo->setObjectName("ImportViewLogo");

//    auto btFrme = new DFrame;
//    auto btFrmeLayout = new QVBoxLayout(btFrme);
//    btFrme->setFixedSize(150, 50);
    d->importButton = new DPushButton;
    d->importButton->setObjectName("ImportViewImportButton");
    d->importButton->setFixedSize(142, 42);
//    btFrmeLayout->addWidget(d->importButton, 0, Qt::AlignCenter);
    d->importButton->setText(tr("Add Music"));

    d->text = new DLabel;
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

    connect(d->importButton, &DPushButton::clicked,
    this, [ = ] {
        Q_EMIT this->importFiles();
    });

    connect(d->text, &DLabel::linkActivated,
    this, [ = ](const QString & /*link*/) {
        showWaitHint();
        Q_EMIT this->scanMusicDirectory();
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

    DFrame::dragEnterEvent(event);
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
        Q_EMIT importSelectFiles(localpaths);
    }
}
