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

class MusicStatckedWidget;
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
class SubSonglistWidget;
class MediaMeta;
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
    // 左上角返回按钮点击
    void slotLeftClicked();
    void slotSearchEditFoucusIn();
    void slotLyricClicked();
    void slotImportFinished(QString hash, int successCount);
    void slotImportFailed();
    // 隐藏二级页面
    void slotHideSubWidget();
    void slotShortCutTriggered();
    void slotMenuTriggered(QAction *action);
    void slotSwitchTheme();
    // 所有歌曲被删除
    void slotAllMusicCleared();
    // 延迟自动播放
    void slotAutoPlay(bool bremb);
    // 文管打开文件
    void slotPlayFromFileMaganager();
    // 显示二级页面
    void slotShowSubSonglist(const QMap<QString, MediaMeta> &musicinfos, bool isAlbumDialog);
protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
private:
    // 控件堆栈
    MusicStatckedWidget  *m_musicStatckedWidget = nullptr;
    // 主页面控件
    MusicContentWidget  *m_musicContentWidget = nullptr;
    // 歌手专辑控件
    SubSonglistWidget   *m_subSonglistWidget = nullptr;
    // 歌词控件
    MusicLyricWidget    *m_musicLyricWidget = nullptr;
    // 标题栏
    DTitlebar           *m_titlebar = nullptr;
    // 标题栏返回按钮
    DPushButton         *m_backBtn = nullptr;
    TitlebarWidget      *m_titlebarwidget = nullptr;
    // 播放控制托盘控件
    FooterWidget        *m_footerWidget = nullptr;
    SearchResult        *m_searchResult = nullptr;
    // 导入控件
    ImportWidget        *m_importWidget = nullptr;
    DequalizerDialog    *m_dequalizerDialog = nullptr;
    // 播放队列控件
    PlayQueueWidget     *m_playQueueWidget = nullptr;
    // 零时消息控件
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

