/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <dsearchedit.h>

class SearchResult;
class SearchEdit : public Dtk::Widget::DSearchEdit
{
    Q_OBJECT
    Q_PROPERTY(QString viewname READ viewname WRITE setViewname NOTIFY viewnameChanged)

public:
    explicit SearchEdit(QWidget *parent = 0);

public:
    void setResultWidget(SearchResult *);
    QString viewname();

signals:
    void viewnameChanged(QString viewname);
    void searchText(const QString &text);
    void locateMusic(const QString &hash);

public slots:
//    void setMode(QString mode);

    void onFocusIn();
    void onFocusOut();
    void onTextChanged();
    void onReturnPressed();

    void setViewname(QString viewname);

protected:
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private:
    SearchResult    *m_result = nullptr;
    QString         m_view;
};
