// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicstackedwidget.h"

#include <QPropertyAnimation>
#include <QDesktopWidget>

#include <DApplication>

#include "ac-desktop-define.h"
#include "commonservice.h"
#include "musicsonglistview.h"

MusicStatckedWidget::MusicStatckedWidget(QWidget *parent)
    : DStackedWidget(parent)
{
    setObjectName("MainWidgetStatcked");
}

void MusicStatckedWidget::animationToUp()
{
    //指明父类防止内存泄露
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos", this);

    animation->setDuration(AnimationDelay);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, 50));
    animation->setEndValue(QPoint(0, -height()));
#ifdef DTKWIDGET_CLASS_DSizeMode
        if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
            animation->setStartValue(QPoint(0, 40));
        }
#endif

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    // 防止挡住歌词控件
    animation->connect(animation, &QPropertyAnimation::finished,
                       this, &MusicStatckedWidget::hide);

//    this->resize(size);
    this->show();

    animation->start();
}

void MusicStatckedWidget::animationToDown()
{
    //指明父类防止内存泄露
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos", this);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setDuration(AnimationDelay);
    animation->setStartValue(QPoint(0, -height()));
    animation->setEndValue(QPoint(0, 50));
#ifdef DTKWIDGET_CLASS_DSizeMode
        if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
            animation->setEndValue(QPoint(0, 40));
        }
#endif

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);

    this->show();

    animation->start();
}

void MusicStatckedWidget::animationImportToDown(const QSize &size)
{
    //初始化指明父类，方便后续释放，防止内存泄露
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos", this);

    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, -size.height()));
    animation->setEndValue(QPoint(0, 50));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);

    this->resize(size);
    this->show();
    animation->start();
}

void MusicStatckedWidget::animationImportToLeft(const QSize &size)
{
    //初始化指明父类，方便后续释放，防止内存泄露
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos", this);

    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, 50));
    animation->setEndValue(QPoint(-size.width(), 50));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    animation->connect(animation, &QPropertyAnimation::finished,
                       this, &MusicStatckedWidget::hide);
    this->resize(size);

    animation->start();
}

void MusicStatckedWidget::animationToUpByInput()
{
    int posY = CommonService::getInstance()->getCurrentWidgetPosY();
    QRect screenRect = DApplication::desktop()->screenGeometry();
    //初始化指明父类，方便后续释放，防止内存泄露
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos", this);
    animation->setDuration(InputAnimationDelay);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, 50));
    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    int keyboardHeight = static_cast<int>(DApplication::inputMethod()->keyboardRectangle().height());
    if (posY > (screenRect.height() - keyboardHeight - MusicSongListView::ItemHeight)) {
        animation->setEndValue(QPoint(0, (screenRect.height() - keyboardHeight - posY)));
        animation->start();
    }
}

void MusicStatckedWidget::animationToDownByInput()
{
    //初始化指明父类，方便后续释放，防止内存泄露
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos", this);

    animation->setDuration(InputAnimationDelay);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(this->pos());
    animation->setEndValue(QPoint(0, 50));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);

    animation->start();
}
