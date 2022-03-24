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

#include "mainframe.h"

#include <QDebug>
#include <QAction>
#include <QProcess>
#include <QStandardPaths>
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QSystemTrayIcon>
#include <QTime>
#include <QShortcut>
#include <QPropertyAnimation>
#include <QDBusConnection>
#include <QDesktopWidget>
#include <QScrollArea>

#include <DUtil>
#include <DWidgetUtil>
#include <DAboutDialog>
#include <DDialog>
#include <DApplication>
#include <DFileDialog>
#include <DHiDPIHelper>
#include <DMessageManager>
#include <DFloatingMessage>
#include <DSettingsDialog>
#include <DSettings>
#include <DThemeManager>
#include <DSettingsWidgetFactory>
#include <DSettingsOption>
#include <DApplicationHelper>
#include <DApplication>

#include <unistd.h>
#include "./core/musicsettings.h"
#include "./core/util/global.h"

#include "../widget/titlebarwidget.h"
#include "musicstackedwidget.h"
#include "musiccontentwidget.h"
#include "footerwidget.h"
#include "searchresult.h"
#include "musiclyricwidget.h"
#include "importwidget.h"
#include "ac-desktop-define.h"
#include "databaseservice.h"
#include "commonservice.h"
#include "shortcut.h"
#include "dequalizerdialog.h"
#include "closeconfirmdialog.h"
#include "playqueuewidget.h"
#include "subsonglistwidget.h"
#include "tabletlabel.h"
#include "comdeepiniminterface.h"
#include "dbusutils.h"
DWIDGET_USE_NAMESPACE

const QString s_PropertyViewname = "viewname";
const QString s_PropertyViewnameLyric = "lyric";
const QString s_PropertyViewnamePlay = "playList";
static DSettingsDialog *equalizer = nullptr;
using namespace Dtk::Widget;

MainFrame::MainFrame()
{
    setObjectName("MainFrame");
    Global::setAppName(tr("Music"));
    QString descriptionText = tr("Music is a local music player with beautiful design and simple functions.");
    QString acknowledgementLink = "https://www.deepin.org/acknowledgments/deepin-music#thanks";
    qApp->setProductName(QApplication::tr("Music"));
    qApp->setApplicationAcknowledgementPage(acknowledgementLink);
    qApp->setProductIcon(QIcon::fromTheme("deepin-music"));
    qApp->setApplicationDescription(descriptionText);
    qApp->setApplicationDisplayName(QApplication::tr("Music"));

    this->setWindowTitle(tr("Music"));

    m_titlebarwidget = new TitlebarWidget(this);
//    m_searchResult = new SearchResult(this);
//    m_titlebarwidget->setResultWidget(m_searchResult);

    m_titlebar = new DTitlebar(this);
    m_titlebar->setFixedHeight(50);
    m_titlebar->setTitle(tr("Music"));
    m_titlebar->setIcon(QIcon::fromTheme("deepin-music"));    //titlebar->setCustomWidget(titlebarwidget, Qt::AlignLeft, false);

    m_titlebar->setCustomWidget(m_titlebarwidget);
    m_titlebar->layout()->setAlignment(m_titlebarwidget, Qt::AlignCenter);
    m_backBtn = new DPushButton(this);
    AC_SET_OBJECT_NAME(m_backBtn, AC_titleBarLeft);
    AC_SET_ACCESSIBLE_NAME(m_backBtn, AC_titleBarLeft);
    m_backBtn->setVisible(false);
    m_backBtn->setFixedSize(QSize(36, 36));

    m_addMusicBtn = new DIconButton(DStyle::SP_IncreaseElement, this);
    m_addMusicBtn->setToolTip(tr("Add music"));
    AC_SET_OBJECT_NAME(m_addMusicBtn, AC_titleBarAddMusic);
    AC_SET_ACCESSIBLE_NAME(m_addMusicBtn, AC_titleBarAddMusic);
    m_addMusicBtn->setFixedSize(QSize(36, 36));

    m_selectStr = tr("Select");
    m_selectAllStr = tr("Select All");
    m_doneStr = tr("Done");
    if (CommonService::getInstance()->isTabletEnvironment()) {
        initTabletSelectBtn();
    }
    m_backBtn->setIcon(QIcon::fromTheme("left_arrow"));
    m_titlebar->addWidget(m_backBtn, Qt::AlignLeft);
    m_titlebar->addWidget(m_addMusicBtn, Qt::AlignRight);
    m_titlebar->resize(width(), 50);

    // 返回按钮点击
    connect(m_backBtn, &DPushButton::clicked,
            this, &MainFrame::slotLeftClicked);

    // 添加音乐
    connect(m_addMusicBtn, &DPushButton::clicked,
            this, &MainFrame::slotAddMusicClicked);
    connect(CommonService::getInstance(), &CommonService::signalAddMusic,
            this, &MainFrame::slotAddMusicClicked);

    connect(m_titlebarwidget, &TitlebarWidget::sigSearchEditFoucusIn,
            this, &MainFrame::slotSearchEditFoucusIn);
    // 导入成功
    connect(DataBaseService::getInstance(), &DataBaseService::signalImportFinished,
            this, &MainFrame::slotDBImportFinished);
    // 导入失败
    connect(DataBaseService::getInstance(), &DataBaseService::signalImportFailed,
            this, &MainFrame::slotImportFailed);
    connect(CommonService::getInstance(), &CommonService::signalShowPopupMessage,
            this, &MainFrame::showPopupMessage);

    connect(DataBaseService::getInstance(), &DataBaseService::signalPlayFromFileMaganager,
            this, &MainFrame::slotPlayFromFileMaganager);

    connect(Player::getInstance()->getMpris(), &MprisPlayer::quitRequested, this, [ = ]() {
        sync();
        qApp->quit();
    });
    connect(Player::getInstance()->getMpris(), &MprisPlayer::raiseRequested, this, [ = ]() {
        if (!isVisible()) {
            restoreGeometry(m_geometryBa);
        }
        // 使用dbus显示窗口
        this->titlebar()->setFocus();
        show();
        raise();
        activateWindow();
    });

    connect(CommonService::getInstance(), &CommonService::signalSwitchToView, this, &MainFrame::slotViewChanged);

    connect(CommonService::getInstance(), &CommonService::signalImprotFromTaskbar, this, [ = ](const QStringList & itemMetas) {
        // 当itemMetas为空时，不做导入
        if (itemMetas.isEmpty())
            return ;
        //初始化导入窗口
        if (m_importWidget == nullptr) {
            DataBaseService::getInstance()->setFirstSong(itemMetas.first());
            DataBaseService::getInstance()->importMedias("all", itemMetas);
        } else {
            m_importWidget->slotFileImportProcessing(itemMetas);
        }
    });

    if (CommonService::getInstance()->isTabletEnvironment()) {
        QDBusConnection connection = QDBusConnection::sessionBus();
        m_comDeepinImInterface = new ComDeepinImInterface("com.deepin.im", "/com/deepin/im", connection);
        if (m_comDeepinImInterface->isValid()) {
            connect(m_comDeepinImInterface, &ComDeepinImInterface::imActiveChanged, this, &MainFrame::slotActiveChanged);
        }
    }
}

MainFrame::~MainFrame()
{
}

void MainFrame::initUI(bool showLoading)
{
    this->setMinimumSize(QSize(900, 600));
    this->setFocusPolicy(Qt::ClickFocus);

    m_titlebarwidget->setEnabled(showLoading);

    /*---------------menu&shortcut-------------------*/
    if (!CommonService::getInstance()->isTabletEnvironment()) {
        initMenuAndShortcut();
        m_newSonglistAction->setEnabled(showLoading);
    } else {
        initPadMenu();
    }

    m_musicContentWidget = new MusicContentWidget(this);
    m_musicContentWidget->setVisible(showLoading);

    m_musicStatckedWidget = new MusicStatckedWidget(this);
    m_musicStatckedWidget->addWidget(m_musicContentWidget);
    m_musicStatckedWidget->setCurrentWidget(m_musicContentWidget);

    m_footerWidget = new FooterWidget(this);
    m_footerWidget->setVisible(showLoading);
    connect(m_footerWidget, &FooterWidget::lyricClicked, this, &MainFrame::slotLyricClicked);

    if (!showLoading) {
        m_importWidget = new ImportWidget(this);
    }
    m_musicLyricWidget = new MusicLyricWidget(this);
    m_musicLyricWidget->hide();

    AC_SET_OBJECT_NAME(m_musicLyricWidget, AC_musicLyricWidget);
    AC_SET_ACCESSIBLE_NAME(m_musicLyricWidget, AC_musicLyricWidget);

    //    m_pwidget = new QWidget(this);

    connect(DataBaseService::getInstance(), &DataBaseService::signalAllMusicCleared,
            this, &MainFrame::slotAllMusicCleared);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &MainFrame::setThemeType);

    // 歌词控件自动收起时更新歌词按钮
    connect(m_musicLyricWidget, &MusicLyricWidget::signalAutoHidden,
            m_footerWidget, &FooterWidget::slotLyricAutoHidden);

    connect(CommonService::getInstance(), &CommonService::signalCdaImportFinished,
            this, &MainFrame::slotCdaImportFinished);

    setThemeType(DGuiApplicationHelper::instance()->themeType());
}

MusicContentWidget *MainFrame::getMusicContentWidget()
{
    return m_musicContentWidget;
}

void MainFrame::initMenuAndShortcut()
{
    //menu
    m_newSonglistAction = new QAction(MainFrame::tr("Add playlist"), this);
    m_newSonglistAction->setEnabled(true);
    m_addMusicFiles = new QAction(MainFrame::tr("Add music"), this);
    m_equalizer = new QAction(MainFrame::tr("Equalizer"), this);
    AC_SET_OBJECT_NAME(m_equalizer, AC_equalizerAction);

    m_settings = new QAction(MainFrame::tr("Settings"), this);
    AC_SET_OBJECT_NAME(m_settings, AC_settingsAction);

    QAction *m_colorModeAction = new QAction(MainFrame::tr("Dark theme"), this);
    m_colorModeAction->setCheckable(true);
    m_colorModeAction->setChecked(2 == DGuiApplicationHelper::instance()->themeType());

    QAction *m_exit = new QAction(MainFrame::tr("Exit"), this);

    DMenu *pTitleMenu = new DMenu(this);

    AC_SET_OBJECT_NAME(pTitleMenu, AC_titleMenu);
    AC_SET_ACCESSIBLE_NAME(pTitleMenu, AC_titleMenu);

    pTitleMenu->addAction(m_newSonglistAction);
    pTitleMenu->addAction(m_addMusicFiles);
    pTitleMenu->addSeparator();
    pTitleMenu->addAction(m_equalizer);
    pTitleMenu->addAction(m_settings);
    pTitleMenu->addSeparator();

    m_titlebar->setMenu(pTitleMenu);

    connect(pTitleMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotMenuTriggered(QAction *)));
    connect(m_colorModeAction, &QAction::triggered, this, &MainFrame::slotSwitchTheme);
    connect(m_exit, SIGNAL(triggered()), this, SLOT(close()));

    //short cut
    addmusicfilesShortcut = new QShortcut(this);
    QKeySequence addmusicKeySequence(QLatin1String("Ctrl+O"));
    addmusicfilesShortcut->setKey(addmusicKeySequence);

    viewshortcut = new QShortcut(this);
    viewshortcut->setAutoRepeat(false);
    QKeySequence viewSequence(QLatin1String("Ctrl+Shift+/"));
    viewshortcut->setKey(viewSequence);

    searchShortcut = new QShortcut(this);
    QKeySequence searchSequence(QLatin1String("Ctrl+F"));
    searchShortcut->setKey(searchSequence);

    windowShortcut = new QShortcut(this);
    QKeySequence windowSequence(QLatin1String("Ctrl+Alt+F"));
    windowShortcut->setKey(windowSequence);

    m_newItemShortcut = new QShortcut(this);
    QKeySequence newItemSequence(QLatin1String("Ctrl+Shift+N"));
    m_newItemShortcut->setKey(newItemSequence);
    connect(m_newItemShortcut, &QShortcut::activated, CommonService::getInstance(), &CommonService::signalAddNewSongList);

    connect(addmusicfilesShortcut, SIGNAL(activated()), this, SLOT(slotShortCutTriggered()));
    connect(viewshortcut, SIGNAL(activated()), this, SLOT(slotShortCutTriggered()));
    connect(searchShortcut, SIGNAL(activated()), this, SLOT(slotShortCutTriggered()));
    connect(windowShortcut, SIGNAL(activated()), this, SLOT(slotShortCutTriggered()));

    //初始化托盘
    auto playAction = new QAction(tr("Play/Pause"), this);
    playAction->setEnabled(DataBaseService::getInstance()->allMusicInfos().size() > 0);
    auto prevAction = new QAction(tr("Previous"), this);
    prevAction->setEnabled(false);
    auto nextAction = new QAction(tr("Next"), this);
    nextAction->setEnabled(false);
    auto quitAction = new QAction(tr("Exit"), this);

    auto trayIconMenu = new DMenu(this);
    trayIconMenu->addAction(playAction);
    trayIconMenu->addAction(prevAction);
    trayIconMenu->addAction(nextAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    //tray icon
    auto m_sysTrayIcon = new QSystemTrayIcon(this);
    m_sysTrayIcon->setIcon(QIcon::fromTheme("deepin-music"));
    m_sysTrayIcon->setToolTip(tr("Music"));
    m_sysTrayIcon->setContextMenu(trayIconMenu);
#ifndef  AUTO_UNIT_TEST
    m_sysTrayIcon->show();
    connect(playAction, &QAction::triggered,
    this, [ = ]() {
        m_footerWidget->slotPlayClick(true);
    });
    connect(prevAction, &QAction::triggered,
    this, [ = ]() {
        m_footerWidget->slotPreClick(true);
    });
    connect(nextAction, &QAction::triggered,
    this, [ = ]() {
        m_footerWidget->slotNextClick(true);
    });
    connect(quitAction, &QAction::triggered,
    this, [ = ]() {
        sync();
        qApp->quit();
    });
    // 歌单数据改变时设置上下一首按钮状态
    connect(Player::getInstance(), &Player::signalPlaylistCountChange, this, [ = ]() {
        prevAction->setEnabled(Player::getInstance()->getPlayList()->size() > 1);
        nextAction->setEnabled(Player::getInstance()->getPlayList()->size() > 1);
    });
    // 清除所有歌曲后置灰播放按钮
    connect(DataBaseService::getInstance(), &DataBaseService::signalAllMusicCleared, this, [ = ]() {
        playAction->setEnabled(Player::getInstance()->getCdaPlayList().size() > 0);
    });
    // 导入歌曲后恢复播放按钮
    connect(DataBaseService::getInstance(), &DataBaseService::signalImportFinished, this, [ = ]() {
        playAction->setEnabled(DataBaseService::getInstance()->allMusicInfos().size() > 0 || Player::getInstance()->getCdaPlayList().size() > 0);
    });
    // 监控cd状态
    connect(CommonService::getInstance(), &CommonService::signalCdaSongListChanged, this, [ = ](int stat) {
        Q_UNUSED(stat)
        playAction->setEnabled(DataBaseService::getInstance()->allMusicInfos().size() > 0 || Player::getInstance()->getCdaPlayList().size() > 0);
        //都不存在时设置未不可操作
        if (Player::getInstance()->getCurrentPlayListHash() == "CdaRole" && DataBaseService::getInstance()->allMusicInfos().isEmpty() && Player::getInstance()->getCdaPlayList().isEmpty()) {
            prevAction->setEnabled(false);
            nextAction->setEnabled(false);
        }
    });
#endif

    connect(m_sysTrayIcon, &QSystemTrayIcon::activated,
    this, [ = ](QSystemTrayIcon::ActivationReason reason) {
        if (QSystemTrayIcon::Trigger == reason) {
            if (isVisible() && !isMinimized()) {
                showMinimized();
            } else {
                if (!isVisible()) {
                    restoreGeometry(m_geometryBa);
                }
                // 使用dbus显示窗口
                this->titlebar()->setFocus();
                show();
                raise();
                activateWindow();
            }
        }
    });
}

void MainFrame::initPadMenu()
{
    // 公有函数防止重复调用引起的内存泄露
    if (m_addMusicFiles || m_select) {
        return;
    }

    m_addMusicFiles = new QAction(MainFrame::tr("Add music"), this);

    m_select = new QAction(m_selectStr, this);
    m_select->setObjectName(AC_tablet_title_select);

    DMenu *pTitleMenu = new DMenu(this);

    AC_SET_OBJECT_NAME(pTitleMenu, AC_titleMenu);
    AC_SET_ACCESSIBLE_NAME(pTitleMenu, AC_titleMenu);

    pTitleMenu->addAction(m_addMusicFiles);
    pTitleMenu->addAction(m_select);

    m_titlebar->setMenu(pTitleMenu);

    connect(pTitleMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotMenuTriggered(QAction *)));
}

void MainFrame::autoStartToPlay()
{
    QString strOpenPath = DataBaseService::getInstance()->getFirstSong();
    auto lastplaypage = MusicSettings::value("base.play.last_playlist").toString(); //上一次的页面
    ListPageSwitchType lastListPageSwitchType = AllSongListType;
    //最后记录的歌单
    if (!lastplaypage.isEmpty()) {
        if (lastplaypage == "album") {
            lastListPageSwitchType = AlbumType;
        } else if (lastplaypage == "artist") {
            lastListPageSwitchType = SingerType;
        } else if (lastplaypage == "fav") {
            lastListPageSwitchType = FavType;
        } else if (lastplaypage == "CdaRole" || lastplaypage == "all"
                   || lastplaypage == "musicResult" || lastplaypage == "artistResult"
                   || lastplaypage == "albumResult") {
            lastListPageSwitchType = AllSongListType;
        } else {
            lastListPageSwitchType = CustomType;
        }
    }
    // 非所有音乐切换歌单
    if (lastListPageSwitchType != AllSongListType)
        emit CommonService::getInstance()->signalSwitchToView(lastListPageSwitchType, lastplaypage);
    Player::getInstance()->reloadMetaList();
    if (!strOpenPath.isEmpty()) {
        //通知设置当前页面
        Player::getInstance()->setCurrentPlayListHash(lastplaypage, false);
        Player::getInstance()->init();
        return ;
    }
    //读取均衡器使能开关配置
    auto eqSwitch = MusicSettings::value("equalizer.all.switch").toBool();
    if (eqSwitch) {
        Player::getInstance()->initEqualizerCfg();
    }
    auto lastMeta = MusicSettings::value("base.play.last_meta").toString();
    if (!lastMeta.isEmpty()) {
        bool bremember = MusicSettings::value("base.play.remember_progress").toBool();
        bool bautoplay = MusicSettings::value("base.play.auto_play").toBool();
        //通知设置当前页面&查询数据
        Player::getInstance()->setCurrentPlayListHash(lastplaypage, false);
        //获取上一次的歌曲信息
        MediaMeta medmeta = DataBaseService::getInstance()->getMusicInfoByHash(lastMeta);
        if (medmeta.localPath.isEmpty()) {
            Player::getInstance()->init();
            return;
        }
        auto playList = Player::getInstance()->getPlayList();
        // 最后播放歌曲默认添加到播放列表
        if (!playList->contains(medmeta)) {
            Player::getInstance()->playListAppendMeta(medmeta);
        }
        Player::getInstance()->setActiveMeta(medmeta);
        if (bremember) {
            //上一次进度的赋值
            medmeta.offset = MusicSettings::value("base.play.last_position").toInt();
            /**
              * 初始不再读取歌曲设置进度，方案更改为直接设置进度，播放歌曲后跳转
              **/
            if (medmeta.offset != 0)
                Player::getInstance()->setPosition(medmeta.offset);
            // 设置进度
            m_footerWidget->slotSetWaveValue(MusicSettings::value("base.play.last_position").toInt(), medmeta.length);
        }
        //加载波形图数据
        m_footerWidget->slotLoadDetector(lastMeta);
        //自动播放处理
        if (bautoplay) {
            slotAutoPlay(medmeta); //不再延迟处理，直接播放
        }
    }
    Player::getInstance()->init();
}

void MainFrame::showPopupMessage(const QString &songListName, int selectCount, int insertCount)
{
    QFontMetrics fm(font());
    QString name = fm.elidedText(songListName, Qt::ElideMiddle, 300);

    auto text = tr("Successfully added to \"%1\"").arg(name); //need translation
    if (selectCount - insertCount > 0) {
        if (selectCount == 1 || insertCount == 0)
            text = tr("Already added to the playlist");
        else {
            if (insertCount == 1)
                text = tr("1 song added");
            else
                text = tr("%1 songs added").arg(insertCount);
        }
    }
    if (selectCount == 0 && insertCount == 0) //cda仅仅添加cd歌单，暂未导入数据，
        text = tr("A disc is connected");

    if (m_popupMessage == nullptr) {
        m_popupMessage = new DWidget(this);
        m_popupMessage->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_popupMessage->setVisible(true);
        m_popupMessage->move(0, 0);
    }

    // 确保弹窗只显示一条
    QList<QWidget *> oldMsgList = m_popupMessage->findChildren<QWidget *>("_d_message_float_deepin_music");
    if (oldMsgList.size() > 0) {
        oldMsgList.first()->deleteLater(); // auto delete
    }
    if (oldMsgList.size() >= 2)
        return;

    QIcon icon = QIcon::fromTheme("notify_success");
    DFloatingMessage *pDFloatingMessage = new DFloatingMessage(DFloatingMessage::MessageType::TransientType, m_popupMessage);
    pDFloatingMessage->setObjectName("_d_message_float_deepin_music");
    pDFloatingMessage->setBlurBackgroundEnabled(true);
    pDFloatingMessage->setMessage(text);
    pDFloatingMessage->setIcon(icon);
    pDFloatingMessage->setDuration(2000);
    m_popupMessage->resize(this->width(), this->height() - m_footerWidget->height());
    m_popupMessage->raise();
    DMessageManager::instance()->sendMessage(m_popupMessage, pDFloatingMessage);
}

void MainFrame::setThemeType(DGuiApplicationHelper::ColorType themeType)
{
    if (getMusicContentWidget()) {
        m_musicContentWidget->slotTheme(themeType);
    }
    if (m_footerWidget != nullptr) {
        m_footerWidget->slotTheme(themeType);
    }
}

void MainFrame::slotLeftClicked()
{
    emit CommonService::getInstance()->signalSwitchToView(PreType, "", QMap<QString, MediaMeta>());
    m_backBtn->setVisible(false);
}

void MainFrame::slotActiveChanged(bool isActive)
{
    if (CommonService::getInstance()->isTabletEnvironment()) {
        if (isActive) {
            m_musicStatckedWidget->animationToUpByInput();
        } else if (m_musicStatckedWidget->pos().y() != 50) {
            m_musicStatckedWidget->animationToDownByInput();
        }
    }
}

void MainFrame::slotSearchEditFoucusIn()
{
    m_titlebarwidget->slotSearchEditFoucusIn();
}

void MainFrame::slotLyricClicked()
{
    // 隐藏播放队列
    if (m_playQueueWidget != nullptr && m_playQueueWidget->isVisible()) {
        playQueueAnimation();
    }
    // 歌词控件显示与关闭动画
    if (m_musicLyricWidget->isHidden()) {
        m_musicLyricWidget->showAnimation();
        m_musicStatckedWidget->animationToUp();
        //搜索页面使能
        m_titlebarwidget->setEnabled(false);
    } else {
        m_musicLyricWidget->closeAnimation();
        m_musicStatckedWidget->animationToDown();
        //搜索页面使能
        m_titlebarwidget->setEnabled(true);
    }
}

void MainFrame::slotDBImportFinished(QString hash, int successCount)
{
    if (successCount <= 0) {
        if (DataBaseService::getInstance()->allMusicInfos().size() <= 0) {
            if (m_importWidget == nullptr) {
                m_importWidget = new ImportWidget(this);
            }
            m_importWidget->showImportHint();
        }
        return;
    }
    // 导入界面显示与关闭动画
    if (m_importWidget && m_importWidget->isVisible()) {
        m_musicStatckedWidget->show();
        m_musicStatckedWidget->animationImportToDown(this->size() - QSize(0, m_footerWidget->height() + titlebar()->height()));
        // 切换到所有音乐界面
        if (hash != "CdaRole")
            emit CommonService::getInstance()->signalSwitchToView(AllSongListType, "all");
        else
            emit CommonService::getInstance()->signalSwitchToView(CdaType, hash); //处理cd加载时，切换到主页面，且没有歌曲存在的情况
        m_footerWidget->show();
        m_importWidget->closeAnimationToDown(this->size());
        if (CommonService::getInstance()->isTabletEnvironment() && m_select) {
            m_select->setEnabled(true);
        }
    }
    m_titlebarwidget->setEnabled(true);
    if (m_newSonglistAction) {
        m_newSonglistAction->setEnabled(true);
    }
}

void MainFrame::slotCdaImportFinished()
{
    if (m_importWidget && m_importWidget->isVisible()) {
        m_musicStatckedWidget->show();
        m_musicStatckedWidget->animationImportToDown(this->size() - QSize(0, m_footerWidget->height() + titlebar()->height()));
        // 切换到所有音乐界面
        emit CommonService::getInstance()->signalSwitchToView(CdaType, "CdaRole"); //处理cd加载时，切换到主页面，且没有歌曲存在的情况
        m_footerWidget->show();
        m_importWidget->closeAnimationToDown(this->size());
        if (CommonService::getInstance()->isTabletEnvironment() && m_select) {
            m_select->setEnabled(true);
        }
    }
    m_titlebarwidget->setEnabled(true);
    m_newSonglistAction->setEnabled(true);
}

void MainFrame::slotImportFailed()
{
    if (DataBaseService::getInstance()->allMusicInfos().size() <= 0) {
        m_importWidget->showImportHint();
    }
    QString message = QString(tr("Import failed, no valid music file found"));
    Dtk::Widget::DDialog warnDlg(this);
    warnDlg.setTextFormat(Qt::RichText);
    warnDlg.setObjectName("uniquewarndailog");
    warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
    //warnDlg.setTitle(message);
    warnDlg.setMessage(message);
    warnDlg.addButton(tr("OK"), true, Dtk::Widget::DDialog::ButtonNormal);
    //warnDlg.setDefaultButton(0);
    warnDlg.exec();
}

void MainFrame::slotShortCutTriggered()
{
    QShortcut *objCut = dynamic_cast<QShortcut *>(sender()) ;
    Q_ASSERT(objCut);

    //添加歌曲时将其加载当前选中歌单里
    if (objCut == addmusicfilesShortcut) {
        slotAddMusicClicked();
    } else if (objCut == viewshortcut) {
        QRect rect = window()->geometry();
        QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
        Shortcut sc;
        QStringList shortcutString;
        QString param1 = "-j=" + sc.toStr();
        QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
        shortcutString << param1 << param2;

        QProcess *shortcutViewProc = new QProcess(this);
        //此处不会造成多进程闲置，deepin-shortcut会自动检查删除多余进程
        shortcutViewProc->startDetached("deepin-shortcut-viewer", shortcutString);

        connect(shortcutViewProc, SIGNAL(finished(int)), shortcutViewProc, SLOT(deleteLater()));
    } else if (objCut == searchShortcut) {
        m_titlebarwidget->slotSearchEditFoucusIn();
    } else if (objCut == windowShortcut) {
        if (windowState() == Qt::WindowMaximized) {
            showNormal();
        } else {
            showMaximized();
        }
    }
}

static DequalizerDialog *dequalizerDialog = nullptr;
static QWidget *createEqualizerWidgetHandle(QObject *opt)
{
    Q_UNUSED(opt)
    dequalizerDialog = new DequalizerDialog(equalizer);
    return dequalizerDialog;
}

void MainFrame::slotMenuTriggered(QAction *action)
{
    Q_ASSERT(action);

    if (action == m_newSonglistAction) {
        // 歌词控件自动收起
        if (!m_musicLyricWidget->isHidden()) {
            m_musicLyricWidget->closeAnimation();
            m_musicStatckedWidget->animationToDown();
            m_titlebarwidget->setEnabled(true);
        }

        emit CommonService::getInstance()->signalAddNewSongList();
    } else if (action == m_addMusicFiles) {
        slotAddMusicClicked();
    } else if (action == m_equalizer) {
        DSettingsDialog *configDialog = new DSettingsDialog(this);
        equalizer = configDialog;
        configDialog->setFixedSize(720, 540);
        AC_SET_OBJECT_NAME(configDialog, AC_Dequalizer);
        AC_SET_ACCESSIBLE_NAME(configDialog, AC_Dequalizer);
        configDialog->widgetFactory()->registerWidget("equalizerWidget", createEqualizerWidgetHandle);

        Dtk::Core::DSettings *equalizerSettings = Dtk::Core::DSettings::fromJsonFile(":/data/dequalizer-settings.json");
        qDebug() << __FUNCTION__ << "" << equalizerSettings->keys();
        //配置均衡器参数
        Player::getInstance()->initEqualizerCfg();
        configDialog->updateSettings(equalizerSettings);
        Dtk::Widget::moveToCenter(configDialog);
        configDialog->setResetVisible(false);
        if (dequalizerDialog && static_cast<QWidget *>(dequalizerDialog->parent())) {
            QWidget *w = static_cast<QWidget *>(dequalizerDialog->parent());

            w->setContentsMargins(0, 0, 0, 0);
            if (w->findChildren<QHBoxLayout *>().size() > 0) {
                for (int i = 0; i < w->findChildren<QHBoxLayout *>().size(); i++) {
                    QHBoxLayout *h = static_cast<QHBoxLayout *>(w->findChildren<QHBoxLayout *>().at(i));
                    if (h) {
                        h->setContentsMargins(0, 0, 0, 0);
                    }
                }
            }
        }
        configDialog->exec();
        delete configDialog;
        delete equalizerSettings;
    } else if (action == m_settings) {
        DSettingsDialog *configDialog = new DSettingsDialog(this);
        AC_SET_OBJECT_NAME(configDialog, AC_configDialog);
        AC_SET_ACCESSIBLE_NAME(configDialog, AC_configDialog);

        configDialog->updateSettings(MusicSettings::settings());
        Dtk::Widget::moveToCenter(configDialog);

        auto curAskCloseAction = MusicSettings::value("base.close.is_close").toBool();
        auto curLastPlaylist = MusicSettings::value("base.play.last_playlist").toString();
        auto curLastMeta = MusicSettings::value("base.play.last_meta").toString();
        auto curLastPosition = MusicSettings::value("base.play.last_position").toInt();

        // 保留均衡器的配置
        int curEqualizerBaud_12K = MusicSettings::value("equalizer.all.baud_12K").toInt();
        int curEqualizerBaud_14K = MusicSettings::value("equalizer.all.baud_14K").toInt();
        int curEqualizerBaud_16K = MusicSettings::value("equalizer.all.baud_16K").toInt();
        int curEqualizerBaud_170 = MusicSettings::value("equalizer.all.baud_170").toInt();
        int curEqualizerBaud_1K = MusicSettings::value("equalizer.all.baud_1K").toInt();
        int curEqualizerBaud_310 = MusicSettings::value("equalizer.all.baud_310").toInt();
        int curEqualizerBaud_3K = MusicSettings::value("equalizer.all.baud_3K").toInt();
        int curEqualizerBaud_60 = MusicSettings::value("equalizer.all.baud_60").toInt();
        int curEqualizerBaud_600 = MusicSettings::value("equalizer.all.baud_600").toInt();
        int curEqualizerBaud_6K = MusicSettings::value("equalizer.all.baud_6K").toInt();
        int curEqualizerBaud_pre = MusicSettings::value("equalizer.all.baud_pre").toInt();
        int curEqualizerCurEffect = MusicSettings::value("equalizer.all.curEffect").toInt();
        bool curEqualizerSwitch = MusicSettings::value("equalizer.all.switch").toBool();

        configDialog->exec();
        delete configDialog;

        MusicSettings::sync();
        MusicSettings::setOption("base.close.is_close", curAskCloseAction);
        MusicSettings::setOption("base.play.last_playlist", curLastPlaylist);
        MusicSettings::setOption("base.play.last_meta", curLastMeta);
        MusicSettings::setOption("base.play.last_position", curLastPosition);

        // 恢复均衡器设置
        MusicSettings::setOption("equalizer.all.baud_12K", curEqualizerBaud_12K);
        MusicSettings::setOption("equalizer.all.baud_14K", curEqualizerBaud_14K);
        MusicSettings::setOption("equalizer.all.baud_16K", curEqualizerBaud_16K);
        MusicSettings::setOption("equalizer.all.baud_170", curEqualizerBaud_170);
        MusicSettings::setOption("equalizer.all.baud_1K", curEqualizerBaud_1K);
        MusicSettings::setOption("equalizer.all.baud_310", curEqualizerBaud_310);
        MusicSettings::setOption("equalizer.all.baud_3K", curEqualizerBaud_3K);
        MusicSettings::setOption("equalizer.all.baud_60", curEqualizerBaud_60);
        MusicSettings::setOption("equalizer.all.baud_600", curEqualizerBaud_600);
        MusicSettings::setOption("equalizer.all.baud_6K", curEqualizerBaud_6K);
        MusicSettings::setOption("equalizer.all.baud_pre", curEqualizerBaud_pre);
        MusicSettings::setOption("equalizer.all.curEffect", curEqualizerCurEffect);
        MusicSettings::setOption("equalizer.all.switch", curEqualizerSwitch);

        //update shortcut
        m_footerWidget->updateShortcut();
        //update fade
        Player::getInstance()->setFadeInOut(MusicSettings::value("base.play.fade_in_out").toBool());
    }

    if (CommonService::getInstance()->isTabletEnvironment()) {
        if (action == m_select) {
            CommonService::getInstance()->setSelectModel(CommonService::MultSelect);
            m_tabletSelectAll->setVisible(true);
            m_tabletSelectDone->setVisible(true);
        }
    }
}

void MainFrame::slotAddMusicClicked()
{
    //初始化导入对话框
    if (m_importWidget == nullptr) {
        m_importWidget = new ImportWidget(this);
    }
    m_importWidget->addMusic(m_importListHash);
}

void MainFrame::slotSwitchTheme()
{
    MusicSettings::setOption("base.play.theme", DGuiApplicationHelper::instance()->themeType());
}

void MainFrame::slotAllMusicCleared()
{
    //当存在cd歌曲时，不返回到初始页面
    if (Player::getInstance()->getCdaPlayList().size() > 0) {
        return;
    }
    qDebug() << "MainFrame::slotAllMusicCleared";
    // 导入界面显示与关闭动画
    m_musicStatckedWidget->animationImportToLeft(this->size() - QSize(0, m_footerWidget->height() + titlebar()->height()));
    m_footerWidget->hide();
    if (m_importWidget == nullptr) {
        m_importWidget = new ImportWidget(this);
    }
    m_importWidget->lower();
    m_importWidget->showImportHint();
    m_importWidget->showAnimationToLeft(this->size());
    if (CommonService::getInstance()->isTabletEnvironment() && m_select) {
        m_select->setEnabled(false);
    }
    m_titlebarwidget->setEnabled(false);
    if (m_newSonglistAction) {
        m_newSonglistAction->setEnabled(false);
    }
}

void MainFrame::slotAutoPlay(const MediaMeta &meta)
{
    qDebug() << "slotAutoPlay=========";
    if (!meta.localPath.isEmpty())
        Player::getInstance()->playMeta(meta);
}

void MainFrame::slotPlayFromFileMaganager()
{
    QString path = DataBaseService::getInstance()->getFirstSong();
    qDebug() << "----------openUrl:" << path << "all size:" << DataBaseService::getInstance()->allMusicInfosCount();
    if (path.isEmpty())
        return;
    //通过路径查询歌曲信息，
    MediaMeta mt = DataBaseService::getInstance()->getMusicInfoByHash(DMusic::filepathHash(path));
    if (mt.localPath.isEmpty()) {
        return;
    }
    Player::getInstance()->playMeta(mt);
    Player::getInstance()->setCurrentPlayListHash("all", true);
    // 通知播放队列列表改变
    emit Player::getInstance()->signalPlayListChanged();
    DataBaseService::getInstance()->setFirstSong("");
}

void MainFrame::slotViewChanged(ListPageSwitchType switchtype, const QString &hashOrSearchword, QMap<QString, MediaMeta> musicinfos)
{
    Q_UNUSED(musicinfos)
    if (switchtype != AlbumSubSongListType
            && switchtype != SingerSubSongListType
            && switchtype != SearchAlbumSubSongListType
            && switchtype != SearchSingerSubSongListType) {
        m_backBtn->setVisible(false);
    }
    // 记录需要导入的歌单hash值
    switch (switchtype) {
    case AlbumType:
    case SingerType:
    case AllSongListType: {
        m_importListHash = "all";
        break;
    }
    case FavType: {
        m_importListHash = "fav";
        break;
    }
    case CustomType: {
        m_importListHash = hashOrSearchword;
        break;
    }
    case SearchMusicResultType:
    case SearchSingerResultType:
    case SearchAlbumResultType:
    case PreType: {
        m_importListHash = "all";
        break;
    }
    case AlbumSubSongListType:
    case SingerSubSongListType:
    case SearchAlbumSubSongListType:
    case SearchSingerSubSongListType: {
        m_backBtn->setVisible(true);
        break;
    }
    default:
        m_importListHash = "all";
        break;
    }
}

void MainFrame::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    if (m_geometryBa.isEmpty()) {//初次直接显示默认窗口
        this->setMinimumSize(QSize(1070, 680));
        this->resize(QSize(1070, 680));
        Dtk::Widget::moveToCenter(this);
    } else {
        QDataStream stream(m_geometryBa); //仅在程序运行期间生效
        stream.setVersion(QDataStream::Qt_4_0);

        const quint32 magicNumber = 0x1D9D0CB;
        quint32 storedMagicNumber;
        stream >> storedMagicNumber;
        if (storedMagicNumber != magicNumber) {
            return;
        }

        const quint16 currentMajorVersion = 2;
        quint16 majorVersion = 0;
        quint16 minorVersion = 0;

        stream >> majorVersion >> minorVersion;

        if (majorVersion > currentMajorVersion) {
            return;
        }

        QRect restoredFrameGeometry;
        QRect restoredNormalGeometry;
        qint32 restoredScreenNumber;
        quint8 maximized;
        quint8 fullScreen;
        //qint32 restoredScreenWidth = 0;

        stream >> restoredFrameGeometry
               >> restoredNormalGeometry
               >> restoredScreenNumber
               >> maximized
               >> fullScreen;

        if (majorVersion > 1) {
            qint32 restoredScreenWidth = 0;
            stream >> restoredScreenWidth;
        }
//        restoreGeometry(m_geometryBa);
    }
    this->setFocus();

    if (m_footerWidget) {
        m_footerWidget->setGeometry(FooterWidget::Margin, height() - FooterWidget::Height - FooterWidget::Margin,
                                    width() - FooterWidget::Margin * 2, FooterWidget::Height);
    }

    if (m_importWidget) {
        // 首页启动导入界面，调整位置
        m_importWidget->setGeometry(0, 0, width(), height());
    }
}

void MainFrame::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        // 窗体状态改变，播放队列收回问题修复
        if (m_playQueueWidget) {
            m_playQueueWidget->stopAnimation();
        }
    }
}

void MainFrame::resizeEvent(QResizeEvent *e)
{
    DMainWindow::resizeEvent(e);
    QSize newSize = DMainWindow::size();

    auto titleBarHeight =  m_titlebar->height();
    m_titlebar->raise();
    m_titlebar->resize(newSize.width(), titleBarHeight);

    if (m_importWidget) {
        m_importWidget->setFixedSize(newSize);
    }

    if (m_footerWidget) {
        m_footerWidget->setGeometry(FooterWidget::Margin, height() - FooterWidget::Height - FooterWidget::Margin,
                                    width() - FooterWidget::Margin * 2, FooterWidget::Height);
    }

    if (m_musicStatckedWidget) {
        m_musicStatckedWidget->setGeometry(0, titlebar()->height(), width(), height() - m_footerWidget->height() - titlebar()->height());
    }

    if (m_musicContentWidget) {
        m_musicContentWidget->setGeometry(0, 0, width(), height() - m_footerWidget->height() - titlebar()->height());
    }

    // 防止主页面控件挡住歌词控件
    if (m_musicLyricWidget) {
        m_musicLyricWidget->setGeometry(0, titlebar()->height(), width(), height() - titlebar()->height());
    }
    // 防止最大化后播放队列高度改变
    if (m_playQueueWidget) {
        m_playQueueWidget->setGeometry(PlayQueueWidget::Margin, size().height() - 450, width() - PlayQueueWidget::Margin * 2, 445);
    }

    if (m_popupMessage) {
        m_popupMessage->resize(this->width(), this->height() - m_footerWidget->height());
    }
    if (CommonService::getInstance()->isTabletEnvironment()) {
        if (this->width() > 1900) {
            CommonService::getInstance()->setIsHScreen(true);
        } else {
            CommonService::getInstance()->setIsHScreen(false);
        }
    }
}

void MainFrame::closeEvent(QCloseEvent *event)
{
    MusicSettings::setOption("base.play.volume", Player::getInstance()->getVolume());
    //保存进度
    auto curPosition = Player::getInstance()->getActiveMeta().offset;
    //是否记录最后播放位置
    if (MusicSettings::value("base.play.remember_progress").toBool()) {
        //防止未播放时重置进度
        if (!Player::getInstance()->getActiveMeta().localPath.isEmpty()) {
            MusicSettings::setOption("base.play.last_position", curPosition);
        }
    } else {
        MusicSettings::setOption("base.play.last_position", -1);
    }
    //当前列表为空转为所有音乐
    QString curPlaylistHash = Player::getInstance()->getCurrentPlayListHash();
    if (curPlaylistHash != "album" && curPlaylistHash != "artist" && !Player::getInstance()->getCurrentPlayListHash().isEmpty()
            && DataBaseService::getInstance()->getPlaylistSongCount(curPlaylistHash) == 0) {
        MusicSettings::setOption("base.play.last_playlist", "all");
    }
    auto askCloseAction = MusicSettings::value("base.close.close_action").toInt();
    switch (askCloseAction) {
    case 0: {
        MusicSettings::setOption("base.close.is_close", false);
        break;
    }
    case 1: {
        MusicSettings::setOption("base.play.state", int(windowState()));
        MusicSettings::setOption("base.close.is_close", true);
        //退出时,stop当前音乐
        Player::getInstance()->stop(false);
        qApp->processEvents();
        qApp->quit();
        break;
    }
    case 2: {
        CloseConfirmDialog ccd(this);
        ccd.setObjectName(AC_CloseConfirmDialog);


        auto clickedButtonIndex = ccd.exec();
        // 1 is confirm button
        if (1 != clickedButtonIndex) {
            event->ignore();
            return;
        }
        if (ccd.isRemember()) {
            MusicSettings::setOption("base.close.close_action", ccd.closeAction());
        }
        if (ccd.closeAction() == 1) {
            MusicSettings::setOption("base.close.is_close", true);
            //退出时,stop当前音乐
            Player::getInstance()->stop(false);
            qApp->processEvents();
            qApp->quit();
        } else {
            MusicSettings::setOption("base.close.is_close", false);
        }
        break;
    }
    default:
        break;
    }

    // 最小化时隐藏关于窗口
    if (qApp->aboutDialog() != nullptr && qApp->aboutDialog()->isVisible()) {
        qApp->aboutDialog()->hide();
    }
    this->setFocus();
    DMainWindow::closeEvent(event);
}

void MainFrame::hideEvent(QHideEvent *event)
{
    //用于最小化时保存窗口位置信息,note：托盘到最小化或者退出程序也会触发该事件
    DMainWindow::hideEvent(event);
    m_geometryBa = saveGeometry();
}

void MainFrame::playQueueAnimation()
{
    if (m_playQueueWidget == nullptr) {
        m_playQueueWidget = new PlayQueueWidget(this);

        AC_SET_OBJECT_NAME(m_playQueueWidget, AC_PlayQueue);
        AC_SET_ACCESSIBLE_NAME(m_playQueueWidget, AC_PlayQueue);

        // 设置播放队列在footer下面
        m_playQueueWidget->raise();
        m_footerWidget->raise();

        connect(m_playQueueWidget, &PlayQueueWidget::signalAutoHidden, m_footerWidget, &FooterWidget::slotPlayQueueAutoHidden);
    }

    m_playQueueWidget->playAnimation(this->size());
}

void MainFrame::initTabletSelectBtn()
{
    // 公有函数防止重复调用引起的内存泄露
    if (m_tabletSelectAll || m_tabletSelectDone) {
        return;
    }

    m_tabletSelectAll = new TabletLabel(m_selectAllStr, m_titlebar, 1);
    m_tabletSelectDone = new TabletLabel(m_doneStr, m_titlebar, 0);
    DFontSizeManager::instance()->bind(m_tabletSelectAll, DFontSizeManager::T6, QFont::Medium);
    DFontSizeManager::instance()->bind(m_tabletSelectDone, DFontSizeManager::T6, QFont::Medium);
    QFontMetrics font(m_tabletSelectAll->font());
    m_tabletSelectAll->setFixedSize((font.width(m_tabletSelectAll->text()) + 22), 50);
    m_tabletSelectDone->setFixedSize((font.width(m_tabletSelectDone->text()) + 22), 50);
    m_tabletSelectAll->hide();
    m_tabletSelectDone->hide();

    m_titlebar->addWidget(m_tabletSelectAll, Qt::AlignRight | Qt::AlignVCenter);
    m_titlebar->addWidget(m_tabletSelectDone, Qt::AlignRight | Qt::AlignVCenter);
    connect(m_tabletSelectAll, &TabletLabel::signalTabletSelectAll, CommonService::getInstance(), &CommonService::signalSelectAll);
    connect(m_tabletSelectDone, &TabletLabel::signalTabletDone, this, [ = ]() {
        CommonService::getInstance()->setSelectModel(CommonService::SingleSelect);
        m_tabletSelectAll->setVisible(false);
        m_tabletSelectDone->setVisible(false);
    });
    connect(CommonService::getInstance(), &CommonService::signalSelectMode, this, [ = ](CommonService::TabletSelectMode mode) {
        if (mode == CommonService::SingleSelect) {
            m_tabletSelectAll->setVisible(false);
            m_tabletSelectDone->setVisible(false);
        }
    });
}


