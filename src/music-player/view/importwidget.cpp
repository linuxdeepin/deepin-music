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
#include <DHiDPIHelper>
#include <DPalette>
#include "musicsettings.h"
DGUI_USE_NAMESPACE

const QString linkTemplate = "<a href='%1'>%2</a>";

class ImportWidgetPrivate
{
public:
    ImportWidgetPrivate(ImportWidget *parent) : q_ptr(parent) {}

    DLabel                  *text = nullptr;
    DPushButton             *importButton = nullptr;
    DLabel                  *logo;
    ImportWidget *q_ptr;
    Q_DECLARE_PUBLIC(ImportWidget)
};

ImportWidget::ImportWidget(QWidget *parent) : DFrame(parent), d_ptr(new ImportWidgetPrivate(this))
{
    Q_D(ImportWidget);
    setObjectName("ImportWidget");

    setAcceptDrops(true);
    setAutoFillBackground(true);

    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);

    d->logo = new DLabel;
    d->logo->setFixedSize(128, 128);
    d->logo->setObjectName("ImportViewLogo");
    d->logo->setPixmap(DHiDPIHelper::loadNxPixmap(":/mpimage/light/import_music.svg"));

    d->importButton = new DPushButton;
    auto importButtonFont = d->importButton->font();
    importButtonFont.setFamily("SourceHanSansSC");
    importButtonFont.setWeight(QFont::Normal);
    importButtonFont.setPixelSize(14);
    d->importButton->setFont(importButtonFont);
    auto pl = d->importButton->palette();
    pl.setColor(DPalette::Dark, QColor("#0098FF"));
    pl.setColor(DPalette::Light, QColor("#25B7FF"));
    pl.setColor(DPalette::ButtonText, QColor("#FFFFFF"));
    QColor sbcolor("#000000");
    sbcolor.setAlphaF(0);
    pl.setColor(DPalette::Shadow, sbcolor);
    d->importButton->setPalette(pl);
    d->importButton->setObjectName("ImportViewImportButton");
    d->importButton->setFixedSize(302, 36);
    d->importButton->setText(tr("Add Music"));
    d->importButton->setFocusPolicy(Qt::NoFocus);

    d->text = new DLabel;
    d->text->setObjectName("ImportViewText");
    auto textFont = d->text->font();
    textFont.setFamily("SourceHanSansSC");
    textFont.setWeight(QFont::Normal);
    textFont.setPixelSize(12);
    d->text->setFont(textFont);
    d->text->setFixedHeight(18);
    QString linkText = QString(linkTemplate).arg(tr("Scan")).arg(tr("Scan"));
    d->text->setText(tr("You can %1 music catalogs or drag and drop music files to add music").arg(linkText));

    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(d->logo, 0, Qt::AlignCenter);
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
    bool themeFlag = false;
    int themeType = MusicSettings::value("base.play.theme").toInt(&themeFlag);
    if (!themeFlag)
        themeType = 1;
    slotTheme(themeType);
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
    d->text->setText(tr("You can %1 music catalogs or drag and drop music files to add music").arg(linkText));
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
void ImportWidget::slotTheme(int type)
{
    Q_D(ImportWidget);
    QString rStr;
    if (type == 1) {
        rStr = "light";
        auto pl = d->importButton->palette();
        pl.setColor(DPalette::Dark, QColor("#0098FF"));
        pl.setColor(DPalette::Light, QColor("#25B7FF"));
        pl.setColor(DPalette::ButtonText, QColor("#FFFFFF"));
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0);
        pl.setColor(DPalette::Shadow, sbcolor);
        d->importButton->setPalette(pl);

        DPalette pa = d->text->palette();
        pa.setColor(DPalette::WindowText, "#777777");
        d->text->setPalette(pa);
        d->text->setForegroundRole(DPalette::WindowText);
    } else {
        rStr = "dark";
        auto pl = d->importButton->palette();
        pl.setColor(DPalette::Dark, QColor("#0056C1"));
        pl.setColor(DPalette::Light, QColor("#004C9C"));
        pl.setColor(DPalette::ButtonText, QColor("#B8D3FF"));
        QColor sbcolor("#0091FF");
        sbcolor.setAlphaF(0.1);
        pl.setColor(DPalette::Shadow, sbcolor);
        d->importButton->setPalette(pl);

        DPalette pa = d->text->palette();
        pa.setColor(DPalette::WindowText, "#798190");
        d->text->setPalette(pa);
        d->text->setForegroundRole(DPalette::WindowText);
    }
    d->logo->setPixmap(DHiDPIHelper::loadNxPixmap(QString(":/mpimage/%1/import_music.svg").arg(rStr)));
}
