// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    leftFrame = new MusicListScrollArea(this);
    leftFrame->setAutoFillBackground(true);
    auto leftFramePalette = leftFrame->palette();
    leftFramePalette.setColor(DPalette::Background, QColor("#FFFFFF"));
    leftFrame->setPalette(leftFramePalette);
    AC_SET_OBJECT_NAME(leftFrame, AC_MusicListScrollArea);
    AC_SET_ACCESSIBLE_NAME(leftFrame, AC_MusicListScrollArea);

    m_listDataWidget = new MusicListDataWidget(this);
    layout->addWidget(leftFrame, 0);
    layout->addWidget(m_listDataWidget, 100);
    AC_SET_OBJECT_NAME(m_listDataWidget, AC_MusicListDataWidget);
    AC_SET_ACCESSIBLE_NAME(m_listDataWidget, AC_MusicListDataWidget);

    slotTheme(DGuiApplicationHelper::instance()->themeType());
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

