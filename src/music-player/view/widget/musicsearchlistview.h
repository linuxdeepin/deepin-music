/*
 * Copyright (C) 2020 chengdu Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <zhaoyongming@uniontech.com>
 *
 * Maintainer: Iceyer <zhaoyongming@uniontech.com>
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

#include "../../core/playlist.h"

#include <DListView>

DWIDGET_USE_NAMESPACE

class MusicSearchListviewPrivate;
class MusicSearchListview: public DListView
{
    Q_OBJECT
public:
    explicit MusicSearchListview(QWidget *parent = Q_NULLPTR);
    ~MusicSearchListview();

    PlaylistPtr playlist() const;
    int rowCount();
    int listSize();
    int getIndexInt()const;
    void onMusiclistChanged(QString text, PlaylistPtr playlist);
    MetaPtrList playMetaList() const;
    PlayMusicTypePtrList playMusicTypePtrList() const;

    void setThemeType(int type);
    int getThemeType() const;
    QString getSearchText()const;

    void setCurrentIndexInt(int row);

    void setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap);
    QPixmap getPlayPixmap() const;
    QPixmap getSidebarPixmap() const;
    QPixmap getAlbumPixmap() const;
    void updateList();
    void keyChoice();

    void setViewModeFlag(QListView::ViewMode mode);
protected:
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<MusicSearchListviewPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusicSearchListview)
};

