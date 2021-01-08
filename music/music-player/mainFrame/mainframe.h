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

#include <QScopedPointer>
#include <DTitlebar>
#include <DWidget>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <DGuiApplicationHelper>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class TitlebarWidget;
class FooterWidget;
class MusicContentWidget;
class SearchResult;
class MusicLyricWidget;
class ImportWidget;
class QShortcut;
class DequalizerDialog;
class PlayQueueWidget;
class QSystemTrayIcon;
class MainFrame : public DMainWindow
{
    Q_OBJECT
public:
    explicit MainFrame();
    ~MainFrame() Q_DECL_OVERRIDE;

    void initUI(bool showLoading);
    MusicContentWidget *getMusicContentWidget();
    // 启动自动播放和进度条设置
    void autoStartToPlay();
    // 播放列表显示与关闭
    void playQueueAnimation();
public slots:
    // 显示弹窗消息
    void showPopupMessage(const QString &songListName, int selectCount, int insertCount);
private:
    /**
     * @brief initMenuAndShortCut
     */
    void initMenuAndShortcut();

private slots:
    void setThemeType(DGuiApplicationHelper::ColorType themeType);
    void slotSearchEditFoucusIn();
    void slotLyricClicked();
    void slotImportFinished(QString hash, int successCount);
    void slotImportFailed();
    void slotShortCutTriggered();
    void slotMenuTriggered(QAction *action);
    void slotSwitchTheme();
    // 所有歌曲被删除
    void slotAllMusicCleared();
    // 延迟自动播放
    void slotAutoPlay(bool bremb);
    // 文管打开文件
    void slotPlayFromFileMaganager();
protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
private:
    DTitlebar           *m_titlebar = nullptr;
    TitlebarWidget      *m_titlebarwidget = nullptr;
    FooterWidget        *m_footerWidget = nullptr;
    MusicContentWidget  *m_musicContentWidget = nullptr;
    SearchResult        *m_searchResult = nullptr;
    MusicLyricWidget    *m_musicLyricWidget = nullptr;
    ImportWidget        *m_importWidget = nullptr;
    DequalizerDialog    *m_dequalizerDialog = nullptr;
    PlayQueueWidget     *m_playQueueWidget = nullptr;
    DWidget             *m_popupMessage = nullptr;
    //mainframe action
    QAction             *m_newSonglistAction    = nullptr;
    QAction             *m_addMusicFiles        = nullptr;
    QAction             *m_equalizer            = nullptr;
    //QAction             *m_colorModeAction      = nullptr;
    //QAction             *m_exit                 = nullptr;
    QAction             *m_settings             = nullptr;
    QString             m_firstPlaySong = "";
    //short cut
    QShortcut           *addmusicfilesShortcut = nullptr;
    QShortcut           *viewshortcut = nullptr;
    QShortcut           *searchShortcut = nullptr;
    QShortcut           *windowShortcut = nullptr;
    //window geometry
    QByteArray          m_geometryBa = QByteArray();
};

//extern const QString s_PropertyViewname;
//extern const QString s_PropertyViewnameLyric;

