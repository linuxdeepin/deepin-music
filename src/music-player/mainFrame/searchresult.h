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

#ifndef SEARCHRESULTLIST_H
#define SEARCHRESULTLIST_H

#include <dabstractdialog.h>
DWIDGET_USE_NAMESPACE

#include <DFrame>
#include <DWidget>
#include <DListView>
#include <DLabel>
#include <DHorizontalLine>
#include <DBlurEffectWidget>
#include <QVBoxLayout>
#include "commonservice.h"

class PushButton;
class QStringListModel;
class MusicSearchListview;
class SearchEdit;
class SearchResult : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit SearchResult(QWidget *parent = Q_NULLPTR);
    ~SearchResult();
    void autoResize();
    void setSearchEdit(SearchEdit *edit);
    //设置各列表搜索关键字
    void setListviewSearchString(const QString &str);
    //点击列表时设置lineEdit显示文字
    void setLineEditSearchString(const QString &str);

    void selectUp();
    void selectDown();
    int  getCurrentIndex();
    ListPageSwitchType getListPageSwitchType();
public:
    // void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    virtual void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;

signals:
    void locateMusic(const QString &hash);

public slots:
    void onReturnPressed();
    void setThemeType(int type);
    //void itemClicked(QModelIndex);
    void getSearchStr();
    //void clearKeyState();

private:
    DLabel *m_MusicLabel;
    DLabel *m_ArtistLabel;
    DLabel *m_AblumLabel;

    DHorizontalLine *s_ArtistLine;
    DHorizontalLine *s_AblumLine;

    MusicSearchListview *m_MusicView    = nullptr;
    MusicSearchListview *m_SingerView   = nullptr;
    MusicSearchListview *m_AlbumView    = nullptr;
    int                 m_CurrentIndex  = -1;
    int                 m_Count         = 0;
    QVBoxLayout         *vlayout        = nullptr;
    SearchEdit          *m_searchEdit   = nullptr;
};
#endif // SEARCHRESULTLIST_H
