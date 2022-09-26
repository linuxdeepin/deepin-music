// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QScopedPointer>
#include <dabstractdialog.h>

#include <DDialog>
#include <DRadioButton>
#include <DCheckBox>

DWIDGET_USE_NAMESPACE

class CloseConfirmDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    enum CloseAction {
        MiniOnClose = 0,
        QuitOnClose = 1,
    };

    explicit CloseConfirmDialog(QWidget *parent = nullptr);
    ~CloseConfirmDialog();

    bool isRemember() const;
    int closeAction() const;

public Q_SLOTS:

private:
    DRadioButton    *m_exitBt     = nullptr;
    DCheckBox       *m_remember   = nullptr;
};

