/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
    void sigFoucusIn();

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
