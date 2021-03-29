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
    void slotSearchEditFoucusIn();
    void slotClearEdit();
signals:
    void sigSearchEditFoucusIn();
protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
public:
    SearchEdit *m_search;
};
