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

#include "closeconfirmdialog.h"

#include <QStyle>
#include <QGroupBox>
#include <QBoxLayout>

#include <DFrame>
#include <DLabel>
#include <DCheckBox>
#include <DRadioButton>
#include <DApplicationHelper>
#include <DFontSizeManager>

#include "core/musicsettings.h"

DWIDGET_USE_NAMESPACE

CloseConfirmDialog::CloseConfirmDialog(QWidget *parent) :
    Dtk::Widget::DDialog(parent)
{
    this->setFixedSize(380, 226);
    this->setSpacing(0);
    DWidget *contentFrame = new DWidget(this);
    contentFrame->setFixedSize(380, 110);
    contentFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setSpacing(10);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    auto miniBt = new DRadioButton(tr("Minimize to system tray"));
    miniBt->setFixedHeight(20);
    miniBt->setObjectName("CloseConfirmDialogMini");
    DFontSizeManager::instance()->bind(miniBt, DFontSizeManager::T6, QFont::Normal);

    m_exitBt = new DRadioButton(tr("Exit"));
    m_exitBt->setFixedHeight(20);
    m_exitBt->setObjectName("CloseConfirmDialogExit");
    DFontSizeManager::instance()->bind(m_exitBt, DFontSizeManager::T6, QFont::Normal);

    m_remember = new DCheckBox(tr("Do not ask again"));
    m_remember->setFixedHeight(20);
    DFontSizeManager::instance()->bind(m_remember, DFontSizeManager::T6, QFont::Normal);

    DLabel *titleLabel = new DLabel;
    titleLabel->setFixedHeight(20);
    titleLabel->setFixedWidth(380);
    titleLabel->setObjectName("MusicListDataTitle");
    titleLabel->setText(tr("Please choose your action"));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setForegroundRole(DPalette::BrightText);
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::T6, QFont::Medium);

    contentLayout->addWidget(titleLabel);
    contentLayout->addWidget(miniBt);
    contentLayout->addWidget(m_exitBt);
    contentLayout->addWidget(m_remember);

    this->addContent(contentFrame, Qt::AlignCenter);
    setIcon(QIcon::fromTheme("deepin-music"));

    addButton(tr("Cancel"), false, ButtonNormal);
    addButton(tr("Confirm"), true, ButtonRecommend);

    m_remember->setChecked(false);
    if (QuitOnClose == MusicSettings::value("base.close.close_action").toInt()) {
        m_exitBt->setChecked(true);
    } else {
        miniBt->setChecked(true);
    }

    style()->polish(this);
}

CloseConfirmDialog::~CloseConfirmDialog()
{

}

bool CloseConfirmDialog::isRemember() const
{
    return m_remember->isChecked();
}

int CloseConfirmDialog::closeAction() const
{
    return m_exitBt->isChecked() ?  QuitOnClose : MiniOnClose;
}
