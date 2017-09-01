/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "content.h"

#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QScrollArea>
#include <QVBoxLayout>

#include <DSettings>
#include <DSettingsGroup>
#include <DSettingsOption>

#include "contenttitle.h"
#include "widgetfactory.h"

namespace dm { //DMUSIC_BEGIN_NAMESPACE

class ContentPrivate
{
public:
    ContentPrivate(Content *parent) : q_ptr(parent)
    {
        widgetFactory = new WidgetFactory;
    }


    QScrollArea         *contentArea;
    QWidget             *contentFrame;
    QVBoxLayout         *contentLayout;

    QMap<QString, QWidget *> titles;

    WidgetFactory       *widgetFactory;

    Content *q_ptr;
    Q_DECLARE_PUBLIC(Content)
};

Content::Content(QWidget *parent) :
    QFrame(parent), d_ptr(new ContentPrivate(this))
{
    Q_D(Content);

    setObjectName("RightFrame");
    setContentsMargins(0, 0, 0, 0);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    d->contentArea = new QScrollArea;
    d->contentArea->setContentsMargins(0, 0, 0, 0);
    d->contentArea->setWidgetResizable(true);
    d->contentArea->setStyleSheet("QScrollArea{border: none; background-color: transparent; }");
//    d->contentArea->setStyleSheet("QFrame{border: 1px solid red;}");

    d->contentFrame = new QWidget(this);
    d->contentFrame->setObjectName("SettingsContent");
    d->contentFrame->setStyleSheet("#SettingsContent{border: none; background-color: transparent; }");
    d->contentLayout = new QVBoxLayout(d->contentFrame);
    d->contentLayout->setContentsMargins(20, 0, 0, 0);
    d->contentLayout->setSpacing(0);
    layout->addWidget(d->contentArea);

    d->contentArea->setWidget(d->contentFrame);

    connect(d->contentArea->verticalScrollBar(), &QScrollBar::valueChanged,
    this, [ = ](int value) {
        Q_D(Content);
        auto offset = std::numeric_limits<int>::max();
        auto closedTitle = d->titles.first();

        for (auto title : d->titles) {
            // not the hide
            if ((title->y() + title->height()) < value) {
                continue;
            }

            auto titleOffset = title->y() + title->height() - value;
            if (titleOffset < offset) {
                offset = titleOffset;
                closedTitle = title;
            }
        }

        if (closedTitle) {
//            qDebug() << "send scrollToGroup" << closedTitle->property("key");
            Q_EMIT scrollToGroup(closedTitle->property("key").toString());
        }
    });
}

Content::~Content()
{

}

WidgetFactory *Content::widgetFactory()
{
    Q_D(Content);
    return d->widgetFactory;
}

void Content::onScrollToGroup(const QString &key)
{
    Q_D(Content);
    auto title = d->titles.value(key);

    this->blockSignals(true);
    d->contentArea->verticalScrollBar()->setValue(title->y());
    this->blockSignals(false);
//    qDebug() << "onScrollToGroup" << key;
}

void Content::updateSettings(QPointer<DTK_CORE_NAMESPACE::DSettings> settings)
{
    Q_D(Content);
    for (auto groupKey : settings->groupKeys()) {
        auto title = new ContentTitle;
        auto trName = QObject::tr(settings->group(groupKey)->name().toStdString().c_str());
        title->setTitle(trName);
        title->setProperty("key", groupKey);
//        d->contentLayout->addSpacing(10);
        d->contentLayout->addWidget(title);
        d->contentLayout->addSpacing(8);
        d->titles.insert(groupKey, title);

        for (auto subgroup : settings->group(groupKey)->childGroups()) {
            if (!subgroup->name().isEmpty()) {
                auto title = new QLabel;
                title->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                title->setObjectName("ContentSubTitleText");
                title->setFixedHeight(20);
                auto trName = QObject::tr(subgroup->name().toStdString().c_str());
                title->setText(trName);
                title->setProperty("key", subgroup->key());
                title->setStyleSheet("#ContentSubTitleText{font-weight: 520; "
//                                     "border: 1px solid red; "
                                     "font-size: 12px;"
                                     "}");
                d->contentLayout->addWidget(title);
                d->contentLayout->addSpacing(10);
                d->titles.insert(subgroup->key(), title);
            }

            for (auto option : subgroup->childOptions()) {
                if (option->isHidden()) {
                    continue;
                }

                auto widget = d->widgetFactory->createWidget(option);
                if (widget) {
                    d->contentLayout->addWidget(widget);
                    widget->setParent(d->contentFrame);
                }
            }

            d->contentLayout->addSpacing(4);
        }
    }

    auto resetBt = new QPushButton(QObject::tr("Restore Defaults"));
    resetBt->setObjectName("SettingsContentReset");
    resetBt->setFixedSize(310, 36);

    d->contentLayout->addSpacing(40);
    d->contentLayout->addWidget(resetBt, 0, Qt::AlignCenter);
    d->contentLayout->addSpacing(40);
    d->contentLayout->addStretch();

    connect(resetBt, &QPushButton::released,
    this, [ = ]() {
        settings->reset();
    });
}

}//DMUSIC_END_NAMESPACE
