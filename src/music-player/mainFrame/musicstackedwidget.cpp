/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

    animation->setDuration(AnimationDelay);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, 50));
    animation->setEndValue(QPoint(0, -height()));

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
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setDuration(AnimationDelay);
    animation->setStartValue(QPoint(0, -height()));
    animation->setEndValue(QPoint(0, 50));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);

    this->show();

    animation->start();
}

void MusicStatckedWidget::animationImportToDown(const QSize &size)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

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
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

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

    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
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
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

    animation->setDuration(InputAnimationDelay);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(this->pos());
    animation->setEndValue(QPoint(0, 50));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);

    animation->start();
}
