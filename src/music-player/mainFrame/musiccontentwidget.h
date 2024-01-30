// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DWidget>
#include <DLabel>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class MusicListDataWidget;
class MusicImageButton;
class MusicListScrollArea;
class MusicContentWidget : public DWidget
{
    Q_OBJECT
public:
    explicit MusicContentWidget(QWidget *parent = Q_NULLPTR);

public slots:
    void slotTheme(DGuiApplicationHelper::ColorType themeType);

protected:
    virtual void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private:
    MusicListDataWidget   *m_listDataWidget;
    MusicListScrollArea   *leftFrame;
    bool                   addFlag = false;
};

