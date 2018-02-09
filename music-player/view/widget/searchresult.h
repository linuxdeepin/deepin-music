/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
