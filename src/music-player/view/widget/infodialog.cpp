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

#include "infodialog.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileInfo>

#include <DApplication>
#include "dplatformwindowhandle.h"
#include "dblureffectwidget.h"
#include <DImageButton>
#include <DFrame>
#include <DFloatingWidget>
#include <DHiDPIHelper>
#include <DWindowCloseButton>

#include <dwindowclosebutton.h>

#include "../../core/metasearchservice.h"
#include "../helper/widgethellper.h"
#include "cover.h"
#include "musicimagebutton.h"

DWIDGET_USE_NAMESPACE

static const int CoverSize = 142;

class InfoDialogPrivate
{
public:
    InfoDialogPrivate(InfoDialog *parent) : q_ptr(parent) {}

    void initUI();
    void initConnection();
    void updateLabelSize();

//    DBlurEffectWidget   *bgBlurWidget   = nullptr;
    DFloatingWidget              *infoGridFrame  = nullptr;
    Cover               *cover          = nullptr;
    DLabel              *title          = nullptr;
    DWindowCloseButton  *closeBt        = nullptr;
    QList<DLabel *>     valueList;

    InfoDialog *q_ptr;
    Q_DECLARE_PUBLIC(InfoDialog)
};

void InfoDialogPrivate::initUI()
{
    Q_Q(InfoDialog);

    q->setObjectName("InfoDialog");
    q->setFixedWidth(320);
//    q->setWindowFlags(q->windowFlags() | Qt::WindowStaysOnTopHint);

    auto layout = new QVBoxLayout(q);
    layout->setSpacing(0);
    layout->setContentsMargins(5, 50, 5, 5);

    closeBt = new DWindowCloseButton( q);
    closeBt->setFixedSize(50, 50);
    closeBt->setIconSize(QSize(50, 50));
    closeBt->move(q->width() - 50, 0);

    cover = new Cover;
    cover->setContentsMargins(0, 0, 0, 0);
    cover->setObjectName("InfoCover");
    cover->setFixedSize(CoverSize, CoverSize);

    title = new DLabel;
    title->setObjectName("InfoTitle");
    title->setFixedWidth(300);
    title->setAlignment(Qt::AlignCenter);
    title->setWordWrap(true);
    title->setForegroundRole(DPalette::TextTitle);

    auto split = new DLabel();
    split->setObjectName("InfoSplit");
    split->setFixedSize(300, 1);

    infoGridFrame = new DFloatingWidget;
    infoGridFrame->setMaximumWidth(300);
    infoGridFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DPalette pl = infoGridFrame->palette();
    QColor windowColor("#FFFFFF");
    windowColor.setAlphaF(0.7);
    pl.setColor(DPalette::Window, windowColor);
    QColor sbcolor("#000000");
    sbcolor.setAlphaF(0.05);
    pl.setColor(DPalette::Shadow, sbcolor);
    infoGridFrame->setPalette(pl);

    //layout->addWidget(closeBt, 0, Qt::AlignTop | Qt::AlignRight);
    layout->addSpacing(10);
    layout->addWidget(cover, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(title, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(split, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(infoGridFrame, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addStretch();

    auto infoLayout = new QVBoxLayout(infoGridFrame);
    infoLayout->setSpacing(0);
    infoLayout->setMargin(5);

    auto basicinfo = new DLabel(InfoDialog::tr("   Basic Information"));
    basicinfo->setMinimumHeight(28);
    basicinfo->setForegroundRole(DPalette::TextTitle);

    auto infogridLayout = new QGridLayout(infoGridFrame);
    infogridLayout->setMargin(10);
    infogridLayout->setHorizontalSpacing(5);
    infogridLayout->setVerticalSpacing(5);
    infogridLayout->setColumnStretch(0, 10);
    infogridLayout->setColumnStretch(1, 100);

    QStringList infoKeys;
    infoKeys << InfoDialog::tr("Title:") << InfoDialog::tr("Artist:")
             << InfoDialog::tr("Album:") << InfoDialog::tr("Type:")
             << InfoDialog::tr("Size:") << InfoDialog::tr("Duration:")
             << InfoDialog::tr("Path:");

    for (int i = 0; i < infoKeys.length(); ++i) {
        auto infoKey = new DLabel(infoKeys.value(i));
        auto infoFont = infoKey->font();
        infoFont.setPointSize(8);
        infoKey->setFont(infoFont);
        infoKey->setObjectName("InfoKey");
        infoKey->setMinimumHeight(18);
        infoKey->setForegroundRole(DPalette::TextTitle);

        auto infoValue = new DLabel();
        infoValue->setFont(infoFont);
        infoValue->setWordWrap(true);
        infoValue->setObjectName("InfoValue");
        infoValue->setMinimumHeight(28);
        infoValue->setMinimumWidth(200);
        infoValue->setMaximumWidth(220);
        infoValue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        infoValue->setForegroundRole(DPalette::TextTitle);
        valueList << infoValue;

        infogridLayout->addWidget(infoKey);
        infogridLayout->addWidget(infoValue);
    }

    infoLayout->addWidget(basicinfo);
    infoLayout->addLayout(infogridLayout);

    q->connect(closeBt, &MusicImageButton::clicked, q, &DAbstractDialog::hide);

//    if (qApp->isDXcbPlatform()) {
//        bgBlurWidget = new DBlurEffectWidget(q);
//        bgBlurWidget->setMaskColor(QColor(255, 255, 255));
//        bgBlurWidget->lower();
//        bgBlurWidget->setBlendMode(DBlurEffectWidget::BehindWindowBlend);
//        bgBlurWidget->setVisible(DPlatformWindowHandle::hasBlurWindow());

//        DPlatformWindowHandle::connectWindowManagerChangedSignal(q, [ = ] {
//            bgBlurWidget->setVisible(DPlatformWindowHandle::hasBlurWindow());
//        });
//    }

}

void InfoDialogPrivate::updateLabelSize()
{
    Q_Q(InfoDialog);
//    auto h = 0;
//    for (auto label : valueList) {
//        label->adjustSize();
//        h += label->size().height() + 6;
//    }
//    infoGridFrame->setFixedHeight(h);
//    infoGridFrame->adjustSize();
//    q->adjustSize();
}

InfoDialog::InfoDialog(QWidget *parent)
    : DAbstractDialog(parent), d_ptr(new InfoDialogPrivate(this))
{
    Q_D(InfoDialog);
    d->initUI();
}

InfoDialog::~InfoDialog()
{

}

void InfoDialog::resizeEvent(QResizeEvent *event)
{
//    Q_D(InfoDialog);
    Dtk::Widget::DAbstractDialog::resizeEvent(event);
//    d->bgBlurWidget->resize(this->size());
}

void InfoDialog::updateInfo(const MetaPtr meta)
{
    Q_D(InfoDialog);
    QString artist = meta->artist.isEmpty() ? tr("Unknown artist") : meta->artist;
    QString album = meta->album.isEmpty() ? tr("Unknown album") : meta->album;
    QStringList infoValues;
    infoValues << meta->title << artist << album
               << meta->filetype << DMusic::sizeString(meta->size) << DMusic::lengthString(meta->length)
               << meta->localPath;

    for (int i = 0; i < d->valueList.length(); ++i) {
        d->valueList.value(i)->setText(infoValues.value(i));
        QFontMetrics fm(d->valueList.value(i)->font());
        QRect rec = fm.boundingRect( d->valueList.value(i)->text());
        int labelRow = d->valueList.value(i)->height() / 14;
        if (rec.width() > d->valueList.value(i)->width() * labelRow) {
            int row = rec.width() / d->valueList.value(i)->width() + 1;
            d->valueList.value(i)->setFixedHeight(row * rec.height());
        }
    }

    QFileInfo fileInfo(meta->localPath);
    QString titleStr(meta->title + "-" + meta->artist + "." + fileInfo.suffix());
    d->title->setText(titleStr);

    auto pixmapSize = static_cast<int>(CoverSize * d->cover->devicePixelRatioF());
    auto coverPixmap = QIcon(":/common/image/info_cover.svg").pixmap(QSize(pixmapSize, pixmapSize));
    QImage cover = coverPixmap.toImage();
    auto coverData = MetaSearchService::coverData(meta);
    if (coverData.length() > 0) {
        cover = QImage::fromData(coverData);
    }
    coverPixmap = QPixmap::fromImage(WidgetHelper::cropRect(cover, QSize(pixmapSize, pixmapSize)));
    coverPixmap.setDevicePixelRatio(d->cover->devicePixelRatioF());
    d->cover->setCoverPixmap(coverPixmap);
    d->updateLabelSize();

    d->title->setFocus();
}

void InfoDialog::setThemeType(int type)
{
    Q_D(InfoDialog);
    QString rStr;
    if (type == 1) {
        rStr = "light";

        DPalette pl = d->infoGridFrame->palette();
        QColor windowColor("#FFFFFF");
        windowColor.setAlphaF(0.7);
        pl.setColor(DPalette::Window, windowColor);
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.05);
        pl.setColor(DPalette::Shadow, sbcolor);
        d->infoGridFrame->setPalette(pl);
    } else {
        rStr = "dark";

        DPalette pl = d->infoGridFrame->palette();
        QColor windowColor("#FFFFFF");
        windowColor.setAlphaF(0.05);
        pl.setColor(DPalette::Window, windowColor);
        QColor sbcolor("#FFFFFF");
        sbcolor.setAlphaF(0.1);
        pl.setColor(DPalette::Shadow, sbcolor);
        d->infoGridFrame->setPalette(pl);
    }
}
