// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DWidget>

DWIDGET_USE_NAMESPACE
class SearchEdit;
class SearchResult;
class TitlebarWidget : public DWidget
{
    Q_OBJECT
public:
    explicit TitlebarWidget(QWidget *parent = Q_NULLPTR);
    ~TitlebarWidget() override;

//    void setResultWidget(SearchResult *result);
public slots:
    void slotSearchEditFocusIn();
    void slotClearEdit();
signals:
    void sigSearchEditFocusIn();
protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
public:
    SearchEdit *m_search;
};
