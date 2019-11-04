/*
 * Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
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

#include "closeconfirmdialog.h"

#include <QStyle>
#include <QGroupBox>
#include <QBoxLayout>

#include <DFrame>
#include <DLabel>
#include <DCheckBox>
#include <DRadioButton>

#include "core/musicsettings.h"

DWIDGET_USE_NAMESPACE

class CloseConfirmDialogPrivate
{
public:
    CloseConfirmDialogPrivate(CloseConfirmDialog *parent) : q_ptr(parent) {}

    DRadioButton    *exitBt     = Q_NULLPTR;
    DCheckBox       *remember   = Q_NULLPTR;

    CloseConfirmDialog *q_ptr;
    Q_DECLARE_PUBLIC(CloseConfirmDialog)
};

CloseConfirmDialog::CloseConfirmDialog(QWidget *parent) :
    Dtk::Widget::DDialog(parent), d_ptr(new CloseConfirmDialogPrivate(this))
{
    Q_D(CloseConfirmDialog);

    auto contentFrame = new DWidget;
    auto contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    auto groupLabel = new DLabel(tr("Please select your operation"));
    groupLabel->setObjectName("CloseConfirmDialogLabel");

    auto groupLabelFont = groupLabel->font();
    groupLabelFont.setFamily("SourceHanSansSC");
    groupLabelFont.setWeight(QFont::Medium);
    groupLabelFont.setPixelSize(14);
    groupLabel->setFont(groupLabelFont);

    auto actionSelectionGroup = new QGroupBox();
    actionSelectionGroup->setContentsMargins(0, 0, 4, 0);
    actionSelectionGroup->setObjectName("CloseConfirmDialogSelectionGroup");

    d->exitBt = new DRadioButton(tr("Exit"));
    d->exitBt->setObjectName("CloseConfirmDialogExit");
    auto miniBt = new DRadioButton(tr("Minimize to system tray"));
    miniBt->setObjectName("CloseConfirmDialogMini");
    auto vbox = new QHBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(d->exitBt);
    vbox->addWidget(miniBt);
    actionSelectionGroup->setLayout(vbox);

    d->remember = new DCheckBox(tr("Do not ask again"));

    auto font = d->exitBt->font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    font.setPixelSize(14);

    d->exitBt->setFont(font);
    miniBt->setFont(font);
    d->remember->setFont(font);

    contentLayout->addWidget(groupLabel, 0, Qt::AlignLeft);
    contentLayout->addSpacing(4);
    contentLayout->addWidget(actionSelectionGroup, 0, Qt::AlignLeft);
    contentLayout->addSpacing(6);
    contentLayout->addWidget(d->remember, 0, Qt::AlignLeft);

    setIcon(QIcon::fromTheme("deepin-music"));
    addContent(contentFrame);

    addButton(tr("Cancel"), false, ButtonNormal);
    addButton(tr("OK"), true, ButtonRecommend);

    d->remember->setChecked(!MusicSettings::value("base.close.ask_close_action").toBool());
    if (QuitOnClose == MusicSettings::value("base.close.close_action").toInt()) {
        d->exitBt->setChecked(true);
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
    Q_D(const CloseConfirmDialog);
    return d->remember->isChecked();
}

int CloseConfirmDialog::closeAction() const
{
    Q_D(const CloseConfirmDialog);
    return d->exitBt->isChecked() ? QuitOnClose : MiniOnClose;
}
