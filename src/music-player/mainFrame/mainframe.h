// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QScopedPointer>
#include <DTitlebar>
#include <DWidget>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <DGuiApplicationHelper>
#include <DMainWindow>
#include "mediameta.h"

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

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
class TabletLabel;
class ComDeepinImInterface;
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
    // 初始化平板选择按钮
    void initTabletSelectBtn();
    // 初始化平板菜单
    void initPadMenu();
    // 检查窗口状态
    bool checkWindowVisible(bool waylandMode);
public slots:
    // 显示弹窗消息
    void showPopupMessage(const QString &songListName, int selectCount, int insertCount);
    // 显示解码失败消息
    void decodingErrorMessage();
    // 左上角返回按钮点击
    void slotLeftClicked();
    // 虚拟键盘
    void slotActiveChanged(bool isActive);
#ifdef DTKWIDGET_CLASS_DSizeMode
    // 紧凑模式
    void slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode);
#endif

private:
    /**
     * @brief initMenuAndShortCut
     */
    void initMenuAndShortcut();
public slots:
    void slotMenuTriggered(QAction *action);
    void slotAddMusicClicked();
    // 导入界面显示与关闭动画
    void slotImportWidgetShowAndClose();

private slots:
    void setThemeType(DGuiApplicationHelper::ColorType themeType);
    void slotSearchEditFocusIn();
    void slotLyricClicked();
    void slotDBImportFinished(QString hash, int successCount);
    void slotCdaImportFinished();
    void slotImportFailed();
    void slotShortCutTriggered();
    void slotSwitchTheme();
    // 所有歌曲被删除
    void slotAllMusicCleared();
    // 延迟自动播放
    void slotAutoPlay(const MediaMeta &meta);
    // 文管打开文件
    void slotPlayFromFileMaganager();
    // 左侧菜单切换ListView
    void slotViewChanged(ListPageSwitchType switchtype, const QString &hashOrSearchword, QMap<QString, MediaMeta> musicinfos);

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* e) Q_DECL_OVERRIDE;

private:
    // 控件堆栈
    MusicStatckedWidget  *m_musicStatckedWidget = nullptr;
    // 主页面控件
    MusicContentWidget   *m_musicContentWidget  = nullptr;
    // 歌词控件
    MusicLyricWidget     *m_musicLyricWidget    = nullptr;
    // 标题栏
    DTitlebar            *m_titlebar            = nullptr;
    // 标题栏返回按钮
    DPushButton          *m_backBtn             = nullptr;
    // 添加音乐按钮
    DIconButton          *m_addMusicBtn         = nullptr;
    TitlebarWidget       *m_titlebarwidget      = nullptr;
    // 标题栏选择控制按钮
    TabletLabel          *m_tabletSelectAll     = nullptr;
    TabletLabel          *m_tabletSelectDone    = nullptr;
    // 播放控制托盘控件
    FooterWidget         *m_footerWidget        = nullptr;
    SearchResult         *m_searchResult        = nullptr;
    // 导入控件
    ImportWidget         *m_importWidget        = nullptr;
    DequalizerDialog     *m_dequalizerDialog    = nullptr;
    // 播放队列控件
    PlayQueueWidget      *m_playQueueWidget     = nullptr;
    // 零时消息控件
    DWidget              *m_popupMessage        = nullptr;

    //mainframe action
    QAction              *m_newSonglistAction    = nullptr;
    QAction              *m_addMusicFiles        = nullptr;
    QAction              *m_equalizer            = nullptr;
    QAction              *m_settings             = nullptr;
    QAction              *m_select               = nullptr;;
    QString               m_firstPlaySong        = "";
    //short cut
    QShortcut            *addmusicfilesShortcut  = nullptr;
    QShortcut            *viewshortcut           = nullptr;
    QShortcut            *searchShortcut         = nullptr;
    QShortcut            *windowShortcut         = nullptr;
    // 新建歌单快捷键
    QShortcut            *m_newItemShortcut      = nullptr;
    //window geometry
    QByteArray            m_geometryBa           = QByteArray();
    // 当前页面hash，供右上角菜单导入使用
    QString               m_importListHash       = "all";
    QString               m_selectStr;
    QString               m_selectAllStr;
    QString               m_doneStr;
    bool                  m_contentUpByKeyBoard  = false;
    bool                  m_preMaxFlag           = false;
    ComDeepinImInterface *m_comDeepinImInterface = nullptr;
    bool                  m_isLyricShow          = false;
};

