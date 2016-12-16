/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef SEARCHEDIT_H
#define SEARCHEDIT_H

#include <dsearchedit.h>

DWIDGET_USE_NAMESPACE

class SearchResult;
class SearchEdit : public DSearchEdit
{
    Q_OBJECT
public:
    explicit SearchEdit(QWidget *parent = 0);

    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
signals:
    void searchText(const QString& text);
    void locateMusic(const QString& hash);

public slots:
    void onFocusIn();
    void onFocusOut();
    void onTextChanged();
    void onReturnPressed();

private:
    SearchResult *m_result = nullptr;
};


#endif // SEARCHEDIT_H
