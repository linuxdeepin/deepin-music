/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dsettingdialog.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QScrollArea>
#include <QMouseEvent>
#include <QCheckBox>
#include <QStyle>
#include <QStyleFactory>

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QPushButton>

#include <dwindowclosebutton.h>
#include "shortcutedit.h"

#include "../helper/thememanager.h"
#include "../../core/dsettings.h"

class DSettingDialogPrivate
{
public:
    DSettingDialogPrivate(DSettingDialog *parent) : q_ptr(parent)
    {
        dsettings = DSettings::instance();
    }

    QFrame              *leftFrame;
    QListWidget         *navbar;
    QVBoxLayout         *leftLayout;


    QFrame              *rightFrame;
    QVBoxLayout         *rightLayout;
    QScrollArea         *content;
    QVBoxLayout         *contentLayout;

    DSettings           *dsettings = nullptr;

    QWidget *createOptionWidget(QJsonObject obj, const QString &key);

    DSettingDialog *q_ptr;
    Q_DECLARE_PUBLIC(DSettingDialog)
};

DSettingDialog::DSettingDialog(QWidget *parent) : ThinWindow(parent), d_ptr(new DSettingDialogPrivate(this))
{
#include <QCheckBox>
    Q_D(DSettingDialog);
    setObjectName("DSettingDialog");

    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    d->leftFrame = new QFrame;
    d->leftFrame->setObjectName("LeftFrame");
    d->leftFrame->setFixedWidth(160);
    d->leftLayout = new QVBoxLayout(d->leftFrame);
    d->leftLayout->setContentsMargins(0, 40, 1, 40);

    d->navbar = new QListWidget;
    d->navbar->setObjectName("Navbar");
    d->navbar->setFixedWidth(159);
    d->leftLayout->addWidget(d->navbar);

    layout->addWidget(d->leftFrame);

    d->rightFrame = new QFrame;
    d->rightFrame->setObjectName("RightFrame");
    d->rightLayout = new QVBoxLayout(d->rightFrame);
    d->rightLayout->setContentsMargins(1, 5, 5, 40);
    d->rightLayout->setSpacing(0);

    d->content = new QScrollArea;
    d->content->setObjectName("Content");
    d->contentLayout = new QVBoxLayout(d->content);

    auto closeBt = new Dtk::Widget::DWindowCloseButton;
    d->rightLayout->addWidget(closeBt, 0, Qt::AlignVCenter | Qt::AlignRight);
    d->rightLayout->addWidget(d->content);

    layout->addWidget(d->rightFrame);
//    layout->addStretch();

    setContentLayout(layout);

    QFile jsonFile("/home/iceyer/Development/deepin/deepin-music/music-player/data/deepin-music-settings.json");
    jsonFile.open(QIODevice::ReadOnly);
    auto jsonDoc = QJsonDocument::fromJson(jsonFile.readAll());
    auto mainGroups = jsonDoc.object().value("main_groups");

    for (auto groupJson : mainGroups.toArray()) {
        auto group = groupJson.toObject();
        auto groupTitle = new NavTitle();
        auto groupKey = group.value("key").toString();

        groupTitle->setText(group.value("name").toString());
        groupTitle->setObjectName("GroupTitle");
        groupTitle->setContentsMargins(30, 0, 0, 0);
        groupTitle->setFixedHeight(30);
        auto item = new QListWidgetItem;
        d->navbar->addItem(item);
        d->navbar->setItemWidget(item, groupTitle);

        auto groubTitle = new ContentTitle(group.value("name").toString());
        d->contentLayout->addWidget(groubTitle);

        for (auto subGroupJson : group.value("groups").toArray()) {
            auto subGroup = subGroupJson.toObject();
            auto subGroupName = subGroup.value("name").toString();
            auto subGroupKey = subGroup.value("key").toString();
            if (!subGroupName.isEmpty()) {
                auto subGroupTitle = new NavSubTitle();
                subGroupTitle->setText(subGroupName);
                subGroupTitle->setContentsMargins(50, 0, 0, 0);
                subGroupTitle->setFixedHeight(30);
                auto item = new QListWidgetItem;
                d->navbar->addItem(item);
                d->navbar->setItemWidget(item, subGroupTitle);
                subGroupTitle->setObjectName("SubGroupTitle");

                auto subGroupContentTitle = new ContentSubTitle();
                subGroupContentTitle->setText(subGroupName);
                d->contentLayout->addWidget(subGroupContentTitle);
            }

            auto options = subGroup.value("options").toArray();

            for (auto optionJson : options) {
                auto option = optionJson.toObject();
                if (option.value("hide").toBool()) {
                    continue;
                }
                auto optionKey = option.value("key").toString();
                auto key = QString("%1.%2.%3").arg(groupKey).arg(subGroupKey).arg(optionKey);
                d->contentLayout->addWidget(d->createOptionWidget(option, key));
            }
        }
    }

    auto resetBt = new QPushButton(tr("Restore to default"));
    resetBt->setObjectName("DSettingDialogReset");
    resetBt->setFixedSize(310, 36);

    d->contentLayout->addSpacing(40);
    d->contentLayout->addWidget(resetBt, 0, Qt::AlignCenter);
    d->contentLayout->addStretch();

    connect(resetBt, &QPushButton::released,
    this, [ = ]() {
        d->dsettings->reset();
    });

    // FIXME: use delagate
    connect(d->navbar, &QListWidget::currentItemChanged,
    this, [ = ](QListWidgetItem * current, QListWidgetItem * previous) {
        auto widget = d->navbar->itemWidget(previous);
        if (widget) {
            widget->setProperty("status", "");
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
            widget->update();
        }
        widget = d->navbar->itemWidget(current);
        if (widget) {
            widget->setProperty("status", "active");
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
            widget->update();
        }
        d->navbar->setCurrentItem(current);
    });

    ThemeManager::instance()->regisetrWidget(this);
    closeBt->setStyle(QStyleFactory::create("dlight"));
    resetBt->setStyle(QStyleFactory::create("dlight"));

    connect(this, &DSettingDialog::mouseMoving, this, &DSettingDialog::moveWindow);
    connect(closeBt, &Dtk::Widget::DWindowCloseButton::clicked, this, &DSettingDialog::close);
}

void DSettingDialog::mouseMoveEvent(QMouseEvent *event)
{
    // TODO: ingore sub control
    Qt::MouseButton button = event->buttons() & Qt::LeftButton ? Qt::LeftButton : Qt::NoButton;
    if (event->buttons() == Qt::LeftButton /*&& d->mousePressed*/) {
        emit mouseMoving(button);
    }
}

DSettingDialog::~DSettingDialog()
{

}

#include <QGridLayout>
QWidget *DSettingDialogPrivate::createOptionWidget(QJsonObject obj, const QString &key)
{
    Q_Q(DSettingDialog);
    auto optWidget = new QFrame;
    optWidget->setObjectName("OptionFrame");

    auto optLayout = new QGridLayout(optWidget);
    optLayout->setContentsMargins(0, 0, 0, 0);
    optLayout->setSpacing(0);
//    optLayout->set(0, 160);

    auto label = obj.value("label").toString();
    auto labelWidget = new QLabel(label);
    labelWidget->setObjectName("OptionLabel");
    optLayout->addWidget(labelWidget, 0, 0, Qt::AlignRight);
    optLayout->setColumnStretch(0, 10);
    optLayout->setColumnStretch(1, 100);
    optLayout->setColumnMinimumWidth(0, 110);
    optLayout->setHorizontalSpacing(20);

    qDebug() << "createOptionWidget" << label;

    auto optType = obj.value("type").toString();
    if (optType == "checkbox") {
        optLayout->setColumnMinimumWidth(0, 110);
        auto value = obj.value("checkbox_text").toString();
        auto optCheckBox = new QCheckBox(value);
        optCheckBox->setObjectName("OptionCheckBox");
        optLayout->addWidget(optCheckBox, 0, 1, Qt::AlignLeft);

        optCheckBox->setChecked(dsettings->option(key).toBool());
        q->connect(optCheckBox, &QCheckBox::stateChanged,
        q, [ = ](int status) {
            dsettings->setOption(key, status == Qt::Checked);
        });
        q->connect(dsettings, &DSettings::optionChange,
        optCheckBox, [ = ](const QString & ckey, const QVariant & value) {
            if (ckey != key) {
                return;
            }
            optCheckBox->setChecked(value.toBool());
        });
        optCheckBox->setStyle(QStyleFactory::create("dlight"));
    }

    if (optType == "shortcut") {
        optLayout->setColumnMinimumWidth(0, 130);
        auto optShortcut = new ShortcutEdit();
        optLayout->addWidget(optShortcut, 0, 1, Qt::AlignLeft);

        optShortcut->setShortCut(dsettings->option(key).toStringList());
        q->connect(optShortcut, &ShortcutEdit::shortcutChanged,
        q, [ = ](QStringList keys) {
            dsettings->setOption(key, (keys));
        });
        q->connect(dsettings, &DSettings::optionChange,
        optShortcut, [ = ](const QString & ckey, const QVariant & value) {
//            qDebug() << ckey << key << value;
            if (ckey != key) {
                return;
            }
            optShortcut->setShortCut(value.toStringList());
            optShortcut->update();
        });
        optShortcut->setStyle(QStyleFactory::create("dlight"));
    }
    return  optWidget;
}

NavSubTitle::NavSubTitle(QWidget *parent): QLabel(parent)
{
    setObjectName("NavSubTitle");
    setProperty("_d_QSSFilename", "DSettingDialog");
    setFixedHeight(30);
    ThemeManager::instance()->regisetrWidget(this);
}

NavTitle::NavTitle(QWidget *parent): QLabel(parent)
{
    setObjectName("NavTitle");
    setProperty("_d_QSSFilename", "DSettingDialog");
    setFixedHeight(30);
    ThemeManager::instance()->regisetrWidget(this);
}

ContentTitle::ContentTitle(const QString &title, QWidget *parent): QFrame(parent)
{
    setObjectName("ContentTitle");
    setProperty("_d_QSSFilename", "DSettingDialog");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    auto titleLabel = new QLabel(title, this);
    titleLabel->setObjectName("ContentTitleText");
    layout->addWidget(titleLabel, 0, Qt::AlignLeft);

    auto lineLabel = new QLabel;
    lineLabel->setObjectName("ContentTitleLine");
    lineLabel->setFixedHeight(2);
    layout->addWidget(lineLabel);
    lineLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(30);
    ThemeManager::instance()->regisetrWidget(this);
}

ContentSubTitle::ContentSubTitle(QWidget *parent): QLabel(parent)
{
    setObjectName("ContentSubTitle");
    setProperty("_d_QSSFilename", "DSettingDialog");
    setFixedHeight(0);
    ThemeManager::instance()->regisetrWidget(this);
}
