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

#pragma once

#include <DScrollArea>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class MusicListView;
class MusicListDataWidget;
class MusicImageButton;

class MusicListScrollArea : public DScrollArea
{
    Q_OBJECT
public:
    explicit MusicListScrollArea(QWidget *parent = Q_NULLPTR);

    MusicListView *getDBMusicListView();
    MusicListView *getCustomMusicListView();
    MusicImageButton *getAddButton();

public slots:
    void slotTheme(int type);
    void changePicture(QPixmap pixmap, QPixmap albumPixmap, QPixmap sidebarPixmap);

protected:
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;

protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    MusicListView         *m_dataBaseListview;
    MusicListView         *m_customizeListview;
    MusicImageButton      *m_addListBtn;
    DLabel                *dataBaseLabel;
    DLabel                *customizeLabel;
};

