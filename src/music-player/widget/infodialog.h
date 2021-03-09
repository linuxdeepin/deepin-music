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

#include <DDialog>
#include <mediameta.h>
#include <DFrame>
#include <DLabel>
#include <DWindowCloseButton>
DWIDGET_USE_NAMESPACE

class Cover;
class InfoDialog : public Dtk::Widget::DAbstractDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(QWidget *parent = Q_NULLPTR);
    ~InfoDialog() override;

    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void updateInfo(const MediaMeta meta);

    void setThemeType(int type);
    void expand(bool expand);
private:
    QString geteElidedText(QFont font, QString str, int MaxWidth);
    void initUI();
    void updateLabelSize();
private:
    DFrame              *infoGridFrame  = nullptr;
    Cover               *cover          = nullptr;
    DLabel              *title          = nullptr;
    DWindowCloseButton  *closeBt        = nullptr;
    DLabel              *basicinfo      = nullptr;
    QList<DLabel *>     valueList;
    QList<DLabel *>     keyList;
    int                 frameHeight     = 0;
    MediaMeta           meta           ;
    bool                DoubleElements  = false;
    bool                isExPand          = true;
};

