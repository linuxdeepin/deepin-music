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

#include <DFrame>
#include <DWidget>
#include <DListView>
#include <DLabel>
#include <DHorizontalLine>
#include <DBlurEffectWidget>
#include <QVBoxLayout>
#include "../core/playlist.h"

class PushButton;
class QStringListModel;
class MusicSearchListview;

class SearchResult : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit SearchResult(QWidget *parent = Q_NULLPTR);
    void autoResize();

    void setSearchString(const QString &str);

    void selectUp();
    void selectDown();

    QString currentStr();

public:
    // void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    virtual void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;

signals:
    void locateMusic(const QString &hash);
    void searchText(const QString  id, const QString &text);
    void searchText2(const QString  id, const QString &text);
    void searchText3(const QString  id, const QString &text);

public slots:
    void onReturnPressed();
    void selectPlaylist(PlaylistPtr playlistPtr);
    void onSearchCand(QString text, PlaylistPtr playlistPtr);
    void slotTheme(int type);
    void itemClicked(QModelIndex);
    void getSearchStr();
    void clearKeyState();

private:
    DLabel *m_MusicLabel;
    DLabel *m_ArtistLabel;
    DLabel *m_AblumLabel;

    DHorizontalLine *s_ArtistLine;
    DHorizontalLine *s_AblumLine;

    PlaylistPtr         playlist        = nullptr;

    MusicSearchListview *m_MusicView    = nullptr;
    MusicSearchListview *m_ArtistView   = nullptr;
    MusicSearchListview *m_AlbumView    = nullptr;
    int                 m_CurrentIndex  = -1;
    int                 m_Count         = 0;
    QVBoxLayout         *vlayout        = nullptr;
    QVBoxLayout         *vlayout1       = nullptr;
    QVBoxLayout         *vlayout2       = nullptr;
    QVBoxLayout         *vlayout3       = nullptr;
};


extern const QString AlbumMusicListID;
extern const QString ArtistMusicListID;
extern const QString AllMusicListID;
extern const QString FavMusicListID;
extern const QString SearchMusicListID;
extern const QString PlayMusicListID;
extern const QString AlbumCandListID;
extern const QString MusicCandListID;
extern const QString ArtistCandListID;

#endif // SEARCHRESULTLIST_H
