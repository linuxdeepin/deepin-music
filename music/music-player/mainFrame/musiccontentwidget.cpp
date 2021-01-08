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

#include "musiccontentwidget.h"

#include <QDebug>

#include <QVBoxLayout>
#include <QFocusEvent>

#include <DListWidget>
#include <DLabel>
#include <DPushButton>
#include <DFloatingButton>
#include <DPalette>
#include <QPropertyAnimation>
#include <DGuiApplicationHelper>
#include <DUtil>

#include "../core/musicsettings.h"

#include "musiclistdatawidget.h"
//#include "widget/musicimagebutton.h"
#include "musiclistscrollarea.h"
#include "ac-desktop-define.h"

static constexpr int AnimationDelay = 400; //ms

MusicContentWidget::MusicContentWidget(QWidget *parent) : DWidget(parent)
{
    setObjectName("MusicListWidget");

    setAutoFillBackground(true);
    auto palette = this->palette();
    palette.setColor(DPalette::Background, QColor("#F8F8F8"));
    setPalette(palette);

    auto layout = new QHBoxLayout(this);
    setFocusPolicy(Qt::ClickFocus);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    leftFrame = new MusicListScrollArea;
    leftFrame->setFixedWidth(220);
    leftFrame->setAutoFillBackground(true);
    auto leftFramePalette = leftFrame->palette();
    leftFramePalette.setColor(DPalette::Background, QColor("#FFFFFF"));
    leftFrame->setPalette(leftFramePalette);
    AC_SET_OBJECT_NAME(leftFrame, AC_MusicListScrollArea);
    AC_SET_ACCESSIBLE_NAME(leftFrame, AC_MusicListScrollArea);

    m_listDataWidget = new MusicListDataWidget;
    layout->addWidget(leftFrame, 0);
    layout->addWidget(m_listDataWidget, 100);
    AC_SET_OBJECT_NAME(m_listDataWidget, AC_MusicListDataWidget);
    AC_SET_ACCESSIBLE_NAME(m_listDataWidget, AC_MusicListDataWidget);

    slotTheme(DGuiApplicationHelper::instance()->themeType());
}

void MusicContentWidget::animationToUp(const QSize &size)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

    animation->setDuration(AnimationDelay);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, 50));
    animation->setEndValue(QPoint(0, -size.height()));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    // 防止挡住歌词控件
    animation->connect(animation, &QPropertyAnimation::finished,
                       this, &MusicContentWidget::hide);

    this->resize(size);
    this->show();

    animation->start();
}

void MusicContentWidget::animationImportToDown(const QSize &size)
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

void MusicContentWidget::animationImportToLeft(const QSize &size)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, 50));
    animation->setEndValue(QPoint(-size.width(), 50));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    animation->connect(animation, &QPropertyAnimation::finished,
                       this, &MusicContentWidget::hide);
    this->resize(size);

    animation->start();
}

void MusicContentWidget::animationToDown(const QSize &size)
{
    this->resize(size);

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

void MusicContentWidget::slotTheme(DGuiApplicationHelper::ColorType themeType)
{
    if (themeType == 1) {
        auto palette = this->palette();
        palette.setColor(DPalette::Background, QColor("#F8F8F8"));
        setPalette(palette);
    } else {
        auto palette = this->palette();
        palette.setColor(DPalette::Background, QColor("#252525"));
        setPalette(palette);
    }

    leftFrame->slotTheme(themeType);
    m_listDataWidget->slotTheme(themeType);
//    m_dataBaseListview->slotTheme(type);
//    m_customizeListview->slotTheme(type);
}


void MusicContentWidget::focusOutEvent(QFocusEvent *event)
{
    // TODO: monitor mouse position
    QPoint mousePos = mapToParent(mapFromGlobal(QCursor::pos()));
    if (!this->geometry().contains(mousePos)) {
        if (event && event->reason() == Qt::MouseFocusReason) {
            DUtil::TimerSingleShot(50, [this]() {
                qDebug() << "self lost focus hide";
//                Q_EMIT this->hidePlaylist();//todo
            });
        }
    }
    DWidget::focusOutEvent(event);
}

void MusicContentWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
//        m_dataBaseListview->clearSelection();
//        m_customizeListview->clearSelection();
    }
    DWidget::keyPressEvent(event);
}

