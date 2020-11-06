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
#include <DGuiApplicationHelper>

#include "musicsettings.h"
DGUI_USE_NAMESPACE

const QString linkTemplate = "<a href='%1'>%2</a>";

class ImportWidgetPrivate
{
public:
    explicit ImportWidgetPrivate(ImportWidget *parent) : q_ptr(parent) {}

    QLabel                  *text = nullptr;
    DPushButton             *importButton = nullptr;
    DPushButton             *addMusicButton = nullptr;
    QLabel                  *logo = nullptr;
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

    d->logo = new QLabel;
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
//    d->importButton->setPalette(pl);
    d->importButton->setObjectName("ImportViewImportButton");
    d->importButton->setFixedSize(302, 36);
    d->importButton->setText(tr("Open Folder"));
    d->importButton->setFocusPolicy(Qt::TabFocus);
    d->importButton->setDefault(true);
    d->importButton->installEventFilter(this);

    d->addMusicButton = new DPushButton;
    d->addMusicButton->setFont(importButtonFont);
//    d->addMusicButton->setPalette(pl);
    d->addMusicButton->setObjectName("ImportViewImportButton");
    d->addMusicButton->setFixedSize(302, 36);
    d->addMusicButton->setText(tr("Add Music"));
    d->addMusicButton->setFocusPolicy(Qt::TabFocus);
    d->addMusicButton->setDefault(true);
    d->addMusicButton->installEventFilter(this);

    d->text = new QLabel;
    d->text->setObjectName("ImportViewText");
    auto textFont = d->text->font();
    textFont.setFamily("SourceHanSansSC");
    textFont.setWeight(QFont::Normal);
    textFont.setPixelSize(12);
    d->text->setFont(textFont);
    d->text->setFixedHeight(18);
    QString linkText = QString(linkTemplate).arg(tr("Scan")).arg(tr("Scan"));
    d->text->setText(tr("%1 music directory or drag music files here").arg(linkText));
    d->text->setFocusPolicy(Qt::TabFocus);
    d->text->installEventFilter(this);

    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(d->logo, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(d->addMusicButton, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(d->importButton, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(d->text, 0, Qt::AlignCenter);
    layout->addStretch();

    connect(d->importButton, &DPushButton::clicked,
    this, [ = ] {
        Q_EMIT this->selectImportDirectory();
    });

    connect(d->addMusicButton, &DPushButton::clicked,
    this, [ = ] {
        Q_EMIT this->importFiles();
    });

    connect(d->text, &DLabel::linkActivated,
    this, [ = ](const QString & /*link*/) {
        showWaitHint();
        Q_EMIT this->scanMusicDirectory();
    });
//    bool themeFlag = false;
//    int themeType = MusicSettings::value("base.play.theme").toInt(&themeFlag);
//    if (!themeFlag)
//        themeType = 1;
    int themeType = DGuiApplicationHelper::instance()->themeType();
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
    d->addMusicButton->hide();
    d->text->setText(tr("Loading music, please wait..."));
}

void ImportWidget::showImportHint()
{
    Q_D(ImportWidget);
    d->importButton->setDisabled(false);
    d->importButton->show();
    d->addMusicButton->show();
    QString linkText = QString(linkTemplate).arg(tr("Scan")).arg(tr("Scan"));
    d->text->setText(tr("%1 music directory or drag music files here").arg(linkText));
}


bool ImportWidget::eventFilter(QObject *o, QEvent *e)
{
    Q_D(ImportWidget);

    if (o == d->text) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *event = static_cast<QKeyEvent *>(e);
            if (event->key() == Qt::Key_Return) {

                Q_EMIT this->scanMusicDirectory();

                auto pe =  d->text->palette();
                pe.setColor(QPalette::WindowText, QColor("#696969"));
                d->text->setPalette(pe);
            }
        } else if (e->type() == QEvent::FocusIn) {

            auto pe =  d->text->palette();
            pe.setColor(QPalette::WindowText, QColor("#A9A9A9"));
            d->text->setPalette(pe);

        } else if (e->type() == QEvent::FocusOut) {

            auto pe =  d->text->palette();
            pe.setColor(QPalette::WindowText, QColor("#696969"));
            d->text->setPalette(pe);
        }
    }

    return QWidget::eventFilter(o, e);
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

void ImportWidget::dragMoveEvent(QDragMoveEvent *e)
{
    DFrame::dragMoveEvent(e);
}

void ImportWidget::dragLeaveEvent(QDragLeaveEvent *e)
{
    DFrame::dragLeaveEvent(e);
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
//        d->importButton->setPalette(pl);

//        d->addMusicButton->setPalette(pl);

        QPalette pa = d->text->palette();
        pa.setColor(QPalette::WindowText, "#777777");
//        d->text->setPalette(pa);
        d->text->setForegroundRole(QPalette::WindowText);
//        d->text->setForegroundRole(DPalette::TextTips);
    } else {
        rStr = "dark";
        auto pl = d->importButton->palette();
        pl.setColor(DPalette::Dark, QColor("#0056C1"));
        pl.setColor(DPalette::Light, QColor("#004C9C"));
        pl.setColor(DPalette::ButtonText, QColor("#B8D3FF"));
        QColor sbcolor("#0091FF");
        sbcolor.setAlphaF(0.1);
        pl.setColor(DPalette::Shadow, sbcolor);
//        d->importButton->setPalette(pl);

//        d->addMusicButton->setPalette(pl);

        QPalette pa = d->text->palette();
        pa.setColor(QPalette::WindowText, "#798190");
//        d->text->setPalette(pa);
        d->text->setForegroundRole(QPalette::WindowText);
//        d->text->setForegroundRole(DPalette::TextTips);
    }
    d->logo->setPixmap(DHiDPIHelper::loadNxPixmap(QString(":/mpimage/%1/import_music.svg").arg(rStr)));
}
