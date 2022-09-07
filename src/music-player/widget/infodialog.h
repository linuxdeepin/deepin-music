// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DDialog>
#include <mediameta.h>
#include <DFrame>
#include <DLabel>
#include <DWindowCloseButton>
DWIDGET_USE_NAMESPACE

class Cover;
class InfoDialog : public Dtk::Widget::DAbstractDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(QWidget *parent = Q_NULLPTR);
    ~InfoDialog() override;

    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void updateInfo(const MediaMeta meta);

    void setThemeType(int type);
    void expand(bool expand);
private:
    QString geteElidedText(QFont font, QString str, int MaxWidth);
    void initUI();
    void updateLabelSize();
private:
    DFrame              *infoGridFrame  = nullptr;
    Cover               *cover          = nullptr;
    DLabel              *title          = nullptr;
    DWindowCloseButton  *closeBt        = nullptr;
    DLabel              *basicinfo      = nullptr;
    QList<DLabel *>     valueList;
    QList<DLabel *>     keyList;
    int                 frameHeight     = 0;
    MediaMeta           meta           ;
    bool                DoubleElements  = false;
    bool                isExPand          = true;
};

