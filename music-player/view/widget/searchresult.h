/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef SEARCHRESULTLIST_H
#define SEARCHRESULTLIST_H

#include <dabstractdialog.h>
DWIDGET_USE_NAMESPACE

#include <QFrame>

class QListView;
class PushButton;
class QStringListModel;

class SearchResult : public QFrame
{
    Q_OBJECT
public:
    explicit SearchResult(QWidget *parent = 0);
    void autoResize();

    void setSearchString(const QString &str);
    void setResultList(const QStringList &titlelist, const QStringList &hashlist);

    bool isSelected();
    void selectUp();
    void selectDown();

    virtual void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
signals:
    void locateMusic(const QString &hash);
    void searchText(const QString &text);

public slots:
    void onReturnPressed();

private:
    QListView           *m_searchResult = nullptr;
    PushButton          *m_doSearchButton = nullptr;
    QStringListModel    *m_model = nullptr;
};

#endif // SEARCHRESULTLIST_H
