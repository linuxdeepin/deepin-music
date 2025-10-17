// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DSearchEdit>

class SearchResult;
class SearchEdit : public Dtk::Widget::DSearchEdit
{
    Q_OBJECT
public:
    explicit SearchEdit(QWidget *parent = Q_NULLPTR);

public:
//    void setResultWidget(SearchResult *);
    void setMainWindow(QWidget *mainWindow);

signals:
    void searchText(const QString &id, const QString &text);
    void locateMusic(const QString &hash);
    void sigFocusIn();

public slots:
    //void onFocusIn();
    void onTextChanged();
    void onReturnPressed();
protected:
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private:
//    void searchText2(QString id, QString text);
//    void searchText3(QString id, QString text);
    SearchResult    *m_result = nullptr;
    QString         m_CurrentId;
    QWidget         *m_mainWindow = nullptr;
//    QString         m_Text;
//    QString         m_LastText;
};
