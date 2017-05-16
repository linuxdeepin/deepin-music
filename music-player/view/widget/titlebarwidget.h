/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QFrame>

class SearchResult;
class TitleBarWidgetPrivate;
class TitleBarWidget : public QFrame
{
    Q_OBJECT
public:
    explicit TitleBarWidget(QWidget *parent = 0);
    ~TitleBarWidget();

    void exitSearch();
    void clearSearch();
    void setSearchEnable(bool enable = true);
    void setResultWidget(SearchResult *);
    void setViewname(const QString &viewname);
signals:
    void searchExited();
    void search(const QString &text);
    void locateMusicInAllMusiclist(const QString &hash);

protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<TitleBarWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), TitleBarWidget)
};
