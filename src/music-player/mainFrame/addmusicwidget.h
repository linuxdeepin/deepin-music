// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QScopedPointer>
#include <DWidget>
#include <DSuggestButton>
#include <QLabel>

DWIDGET_USE_NAMESPACE

class AddMusicWidget : public DWidget
{
    Q_OBJECT
public:
    explicit AddMusicWidget(QWidget *parent = Q_NULLPTR);
    ~AddMusicWidget() override;

public slots:
    void setThemeType(int type);
    void slotAddMusicButtonClicked();
    void setSongList(const QString &hash);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private:
    QLabel                  *m_text = nullptr;
    DSuggestButton          *m_addMusicButton = nullptr;
    QLabel                  *m_logo = nullptr;
    QString                  m_currentHash = "all";
};

