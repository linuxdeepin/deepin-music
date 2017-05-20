/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "infodialog.h"

#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>

#include <DApplication>
#include "dplatformwindowhandle.h"
#include "dblureffectwidget.h"

#include <dwindowclosebutton.h>
#include <thememanager.h>

#include "../../core/metasearchservice.h"
#include "../helper/widgethellper.h"

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
    QFrame              *infoGridFrame  = nullptr;
    QLabel              *cover          = nullptr;
    QLabel              *title          = nullptr;
    QList<QLabel *>     valueList;

    InfoDialog *q_ptr;
    Q_DECLARE_PUBLIC(InfoDialog)
};


void InfoDialogPrivate::initUI()
{
    Q_Q(InfoDialog);

    q->setObjectName("InfoDialog");
    q->setFixedWidth(320);
    q->setWindowFlags(q->windowFlags() | Qt::WindowStaysOnTopHint);

    auto layout = new QVBoxLayout(q);
    layout->setSpacing(0);
    layout->setMargin(5);

    auto closeBt = new DWindowCloseButton;
    closeBt->setObjectName("InfoClose");
    closeBt->setFixedSize(27, 23);

    cover = new QLabel;
    cover->setContentsMargins(0, 0, 0, 0);
    cover->setObjectName("InfoCover");
    cover->setFixedSize(CoverSize, CoverSize);

    title = new QLabel;
    title->setObjectName("InfoTitle");
    title->setFixedWidth(300);
    title->setWordWrap(true);

    auto split = new QLabel();
    split->setObjectName("InfoSplit");
    split->setFixedSize(300, 1);

    infoGridFrame = new QFrame;
    infoGridFrame->setMaximumWidth(300);
    infoGridFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(closeBt, 0, Qt::AlignTop | Qt::AlignRight);
    layout->addSpacing(43);
    layout->addWidget(cover, 0, Qt::AlignCenter);
    layout->addSpacing(13);
    layout->addWidget(title, 0, Qt::AlignCenter);
    layout->addSpacing(19);
    layout->addWidget(split, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(infoGridFrame, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addStretch();

    auto infogridLayout = new QGridLayout(infoGridFrame);
    infogridLayout->setMargin(0);
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
        auto infoKey = new QLabel(infoKeys.value(i));
        infoKey->setObjectName("InfoKey");
        infoKey->setMinimumHeight(18);

        auto infoValue = new QLabel();
        infoValue->setWordWrap(true);
        infoValue->setObjectName("InfoValue");
        infoValue->setMinimumHeight(18);
        infoValue->setMinimumWidth(200);
        infoValue->setMaximumWidth(220);
        infoValue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        infoValue->adjustSize();
        valueList << infoValue;

        infogridLayout->addWidget(infoKey);
        infogridLayout->addWidget(infoValue);
    }

    q->connect(closeBt, &DWindowCloseButton::clicked, q, &DAbstractDialog::hide);

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
    auto h = 0;
    for (auto label : valueList) {
        label->adjustSize();
        h += label->size().height() + 6;
    }
    infoGridFrame->setFixedHeight(h);
    infoGridFrame->adjustSize();
    q->adjustSize();
}

InfoDialog::InfoDialog(QWidget *parent)
    : DAbstractDialog(parent), d_ptr(new InfoDialogPrivate(this))
{
    Q_D(InfoDialog);
    ThemeManager::instance()->regisetrWidget(this);
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
    }

    d->title->setText(meta->title);

    auto coverPixmap = QPixmap(":/common/image/info_cover.png");
    auto coverData = MetaSearchService::coverData(meta);
    if (coverData.length() > 0) {
        QImage cover;
        cover = QImage::fromData(coverData);
        coverPixmap = QPixmap::fromImage(WidgetHelper::cropRect(cover, QSize(CoverSize, CoverSize)));
    }
    d->cover->setPixmap(coverPixmap.scaled(CoverSize, CoverSize));
    d->updateLabelSize();

    d->title->setFocus();
}
