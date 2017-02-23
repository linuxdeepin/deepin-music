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

#include <dwindowclosebutton.h>
#include <thememanager.h>

#include "../../core/metasearchservice.h"
#include "../helper/widgethellper.h"

DWIDGET_USE_NAMESPACE

class InfoDialogPrivate
{
public:
    InfoDialogPrivate(InfoDialog *parent) : q_ptr(parent) {}

    void initUI();
    void initConnection();
    void updateLabelSize();

    QFrame *m_infogridFrame = nullptr;
    QList<QLabel *> m_valueList;
    QLabel *m_cover = nullptr;
    QLabel *m_title = nullptr;

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
//    closeBt->setAttribute(Qt::WA_NoMousePropagation);

    m_cover = new QLabel;
    m_cover->setContentsMargins(0, 0, 0, 0);
    m_cover->setObjectName("InfoCover");
    m_cover->setFixedSize(140, 140);

    m_title = new QLabel;
    m_title->setObjectName("InfoTitle");
    m_title->setFixedWidth(300);
    m_title->setWordWrap(true);

    auto split = new QLabel();
    split->setObjectName("InfoSplit");
    split->setFixedSize(300, 1);

    m_infogridFrame = new QFrame;
    m_infogridFrame->setMaximumWidth(300);
    m_infogridFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(closeBt, 0, Qt::AlignTop | Qt::AlignRight);
    layout->addSpacing(43);
    layout->addWidget(m_cover, 0, Qt::AlignCenter);
    layout->addSpacing(13);
    layout->addWidget(m_title, 0, Qt::AlignCenter);
    layout->addSpacing(19);
    layout->addWidget(split, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(m_infogridFrame, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addStretch();

    auto infogridLayout = new QGridLayout(m_infogridFrame);
    infogridLayout->setMargin(0);
    infogridLayout->setHorizontalSpacing(5);
    infogridLayout->setVerticalSpacing(5);
    infogridLayout->setColumnStretch(0, 10);
    infogridLayout->setColumnStretch(1, 100);

    QStringList infoKeys;
    infoKeys << InfoDialog::tr("Title:") << InfoDialog::tr("Artist:")
             << InfoDialog::tr("Album:") << InfoDialog::tr("File type:")
             << InfoDialog::tr("Size:") << InfoDialog::tr("Length:")
             << InfoDialog::tr("Directory:");


    for (int i = 0; i < infoKeys.length(); ++i) {
        auto infoKey = new QLabel(infoKeys.value(i));
        infoKey->setObjectName("InfoKey");
        infoKey->setMinimumHeight(18);
//        infoKey->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

        auto infoValue = new QLabel(/*infoValues.value(i)*/);
        infoValue->setWordWrap(true);
        infoValue->setObjectName("InfoValue");
        infoValue->setMinimumHeight(18);
//        infoValue->setContentsMargins(5,0,5,0);
        infoValue->setMinimumWidth(200);
        infoValue->setMaximumWidth(220);
        infoValue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        infoValue->adjustSize();
        m_valueList << infoValue;

        infogridLayout->addWidget(infoKey);
        infogridLayout->addWidget(infoValue);
    }

    q->connect(closeBt, &DWindowCloseButton::clicked, q, &DAbstractDialog::hide);
}

void InfoDialogPrivate::updateLabelSize()
{
    Q_Q(InfoDialog);
    auto h = 0;
    for (auto label : m_valueList) {
        label->adjustSize();
        h += label->size().height() + 6;
    }
    m_infogridFrame->setFixedHeight(h);
    m_infogridFrame->adjustSize();
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

void InfoDialog::updateInfo(const MetaPtr meta)
{
    Q_D(InfoDialog);
    QString artist = meta->artist.isEmpty() ? tr("Unkonw artist") : meta->artist;
    QString album = meta->album.isEmpty() ? tr("Unkonw album") : meta->album;
    QStringList infoValues;
    infoValues << meta->title << artist << album
               << meta->filetype << DMusic::sizeString(meta->size) << DMusic::lengthString(meta->length)
               << meta->localPath;

    for (int i = 0; i < d->m_valueList.length(); ++i) {
        d->m_valueList.value(i)->setText(infoValues.value(i));
    }

    d->m_title->setText(meta->title);

    auto coverPixmap = QPixmap(":/common/image/info_cover.png");
    auto coverData = MetaSearchService::coverData(meta);
    if (coverData.length() > 0) {
        QImage cover;
        cover = QImage::fromData(coverData);
        coverPixmap = QPixmap::fromImage(WidgetHelper::cropRect(cover, QSize(140, 140)));
    }
    d->m_cover->setPixmap(coverPixmap);
    d->updateLabelSize();

    d->m_title->setFocus();
}
