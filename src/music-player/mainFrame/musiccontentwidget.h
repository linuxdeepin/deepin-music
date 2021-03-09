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
#include <DLabel>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE

class MusicListDataWidget;
class MusicImageButton;
class MusicListScrollArea;
class MusicContentWidget : public DWidget
{
    Q_OBJECT
public:
    explicit MusicContentWidget(QWidget *parent = Q_NULLPTR);
public slots:
    void slotTheme(DGuiApplicationHelper::ColorType themeType);
protected:
    virtual void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private:
    MusicListDataWidget   *m_listDataWidget;
//    MusicImageButton      *m_addListBtn;
    MusicListScrollArea   *leftFrame;
    bool                   addFlag = false;
};

