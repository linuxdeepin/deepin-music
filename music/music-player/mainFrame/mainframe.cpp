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
#include <QShortcut>
#include <QPropertyAnimation>
#include <DThemeManager>

#include <unistd.h>

#include "./core/musicsettings.h"
#include "./core/util/global.h"

#include "../widget/titlebarwidget.h"
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
DWIDGET_USE_NAMESPACE

const QString s_PropertyViewname = "viewname";
const QString s_PropertyViewnameLyric = "lyric";
const QString s_PropertyViewnamePlay = "playList";

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

    this->setWindowTitle(tr("Music"));

    m_titlebarwidget = new TitlebarWidget(this);
    m_searchResult = new SearchResult(this);
    m_titlebarwidget->setResultWidget(m_searchResult);

    m_titlebar = new DTitlebar(this);
    m_titlebar->setFixedHeight(50);
    m_titlebar->setTitle(tr("Music"));
    m_titlebar->setIcon(QIcon::fromTheme("deepin-music"));    //titlebar->setCustomWidget(titlebarwidget, Qt::AlignLeft, false);

    m_titlebar->setCustomWidget(m_titlebarwidget);
    m_titlebar->layout()->setAlignment(m_titlebarwidget, Qt::AlignCenter);
    m_titlebar->resize(width(), 50);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &MainFrame::slotTheme);
    QObject::connect(m_titlebarwidget, &TitlebarWidget::sigSearchEditFoucusIn,
                     this, &MainFrame::slotSearchEditFoucusIn);
    QObject::connect(DataBaseService::getInstance(), &DataBaseService::sigImportFinished,
                     this, &MainFrame::slotImportFinished);
    QObject::connect(CommonService::getInstance(), &CommonService::showPopupMessage,
                     this, &MainFrame::showPopupMessage);

    connect(Player::instance()->getMpris(), &MprisPlayer::quitRequested, this, [ = ]() {
        sync();
        qApp->quit();
    });
    connect(Player::instance()->getMpris(), &MprisPlayer::raiseRequested, this, [ = ]() {
        if (isVisible()) {
            if (isMinimized()) {
                if (isFullScreen()) {
                    hide();
                    showFullScreen();
                } else {
                    this->titlebar()->setFocus();
                    showNormal();
                    activateWindow();
                    auto geometry = MusicSettings::value("base.play.geometry").toByteArray();
                    this->restoreGeometry(geometry);
                }
            } else {
                showMinimized();
                hide();
            }
        } else {
            this->titlebar()->setFocus();
            showNormal();
            activateWindow();
        }
    });
}

MainFrame::~MainFrame()
{
    MusicSettings::sync();
    MusicSettings::setOption("base.play.state", saveState());
    MusicSettings::setOption("base.play.geometry", saveGeometry());
}

void MainFrame::initUI(bool showLoading)
{
    this->setMinimumSize(QSize(900, 600));
    this->setFocusPolicy(Qt::ClickFocus);

    m_titlebarwidget->setEnabled(showLoading);

//    m_centralWidget = new QWidget(this);
//    m_contentLayout = new QStackedLayout(m_centralWidget);
//    m_contentLayout->setContentsMargins(0, 0, 0, 5);
//    this->setCentralWidget(m_centralWidget);

    m_musicContentWidget = new MusicContentWidget(this);
    m_musicContentWidget->setVisible(showLoading);

    m_footer = new FooterWidget(this);
    m_footer->setVisible(showLoading);
    connect(m_footer, SIGNAL(lyricClicked()), this, SLOT(slotLyricClicked()));

    m_importWidget = new ImportWidget(this);
    m_importWidget->setVisible(!showLoading);


    m_musicLyricWidget = new MusicLyricWidget(this);
    m_musicLyricWidget->hide();

    connect(Player::instance(), &Player::signalMediaMetaChanged,
            m_musicLyricWidget, &MusicLyricWidget::onCoverChanged);
    AC_SET_OBJECT_NAME(m_musicLyricWidget, AC_musicLyricWidget);
    AC_SET_ACCESSIBLE_NAME(m_musicLyricWidget, AC_musicLyricWidget);

    //    m_pwidget = new QWidget(this);

    /*---------------menu&shortcut-------------------*/
    initMenuAndShortcut();

    connect(DataBaseService::getInstance(), &DataBaseService::sigAllMusicCleared,
            this, &MainFrame::slotAllMusicCleared);
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
    connect(m_colorModeAction, SIGNAL(triggered()), this, SLOT(slotSwitchTheme()));
    connect(m_exit, SIGNAL(triggered()), this, SLOT(close()));

    //short cut
    addmusicfilesShortcut = new QShortcut(this);
    addmusicfilesShortcut->setKey(QKeySequence(QLatin1String("Ctrl+O")));

    viewshortcut = new QShortcut(this);
    viewshortcut->setKey(QKeySequence(QLatin1String("Ctrl+Shift+/")));

    searchShortcut = new QShortcut(this);
    searchShortcut->setKey(QKeySequence(QLatin1String("Ctrl+F")));

    windowShortcut = new QShortcut(this);
    windowShortcut->setKey(QKeySequence(QLatin1String("Ctrl+Alt+F")));

    connect(addmusicfilesShortcut, SIGNAL(activated()), this, SLOT(slotShortCutTriggered()));
    connect(viewshortcut, SIGNAL(activated()), this, SLOT(slotShortCutTriggered()));
    connect(searchShortcut, SIGNAL(activated()), this, SLOT(slotShortCutTriggered()));
    connect(windowShortcut, SIGNAL(activated()), this, SLOT(slotShortCutTriggered()));

    //初始化托盘
    auto playAction = new QAction(tr("Play/Pause"), this);
    auto prevAction = new QAction(tr("Previous"), this);
    auto nextAction = new QAction(tr("Next"), this);
    auto quitAction = new QAction(tr("Exit"), this);

    auto trayIconMenu = new DMenu(this);
    trayIconMenu->addAction(playAction);
    trayIconMenu->addAction(prevAction);
    trayIconMenu->addAction(nextAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    auto trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon::fromTheme("deepin-music"));
    trayIcon->setToolTip(tr("Music"));
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();

    connect(playAction, &QAction::triggered,
    this, [ = ]() {
        m_footer->slotPlayClick(true);
    });
    connect(prevAction, &QAction::triggered,
    this, [ = ]() {
        m_footer->slotPreClick(true);
    });
    connect(nextAction, &QAction::triggered,
    this, [ = ]() {
        m_footer->slotNextClick(true);
    });
    connect(quitAction, &QAction::triggered,
    this, [ = ]() {
        sync();
        qApp->quit();
    });

    connect(trayIcon, &QSystemTrayIcon::activated,
    this, [ = ](QSystemTrayIcon::ActivationReason reason) {
        if (QSystemTrayIcon::Trigger == reason) {
            if (isVisible()) {
                if (isMinimized()) {
                    if (isFullScreen()) {
                        hide();
                        showFullScreen();
                    } else {
                        this->titlebar()->setFocus();
                        showNormal();
                        activateWindow();
                        auto geometry = MusicSettings::value("base.play.geometry").toByteArray();
                        this->restoreGeometry(geometry);
                    }
                } else {
                    showMinimized();
                    hide();
                }
            } else {
                this->titlebar()->setFocus();
                showNormal();
                activateWindow();
            }
        }
    });
}

void MainFrame::autoStartToPlay()
{
    QUrl strOpenUrl = MusicSettings::value("base.play.to_open_uri").toString();
    auto lastplaypage = MusicSettings::value("base.play.last_playlist").toString(); //上一次的页面
    if (!strOpenUrl.isEmpty()) {
        //通知设置当前页面
        Player::instance()->setCurrentPlayListHash(lastplaypage, true);
        qDebug() << "lastplaypage:========" << lastplaypage;
        //设置当前歌曲文件
        m_firstPlaySong = strOpenUrl.toLocalFile();
        connect(DataBaseService::getInstance(), &DataBaseService::sigPlayFromFileMaganager, this, &MainFrame::slotPlayFromFileMaganager);
        MusicSettings::setOption("base.play.to_open_uri", "");
        MusicSettings::sync();
        return ;
    }
    auto lastMeta = MusicSettings::value("base.play.last_meta").toString();
    if (!lastMeta.isEmpty()) {
        bool bremb = MusicSettings::value("base.play.remember_progress").toBool();
        bool bautoplay = MusicSettings::value("base.play.auto_play").toBool();
        //通知设置当前页面&查询数据
        Player::instance()->setCurrentPlayListHash(lastplaypage, true);
        //获取上一次的歌曲信息
        MediaMeta medmeta = DataBaseService::getInstance()->getMusicInfoByHash(lastMeta);
        if (medmeta.localPath.isEmpty())
            return;
        if (bremb) {
            Player::instance()->setActiveMeta(medmeta);
            //加载进度
            Player::instance()->loadMediaProgress(medmeta.localPath);
            //设置进度
            QTimer::singleShot(150, [ = ]() {
                Player::instance()->setPosition(MusicSettings::value("base.play.last_position").toInt());
            });
            //加载波形图数据
            m_footer->slotLoadDetector(lastMeta);
        }

        //自动播放处理
        if (bautoplay) {
            QTimer::singleShot(200, [ = ]() {
                slotAutoPlay(bremb);
            });
        }
    }
}

void MainFrame::showPopupMessage(const QString &songListName, int selectCount, int insertCount)
{
    QFontMetrics fm(font());
    QString name = fm.elidedText(songListName, Qt::ElideMiddle, 300);

    auto text = tr("Successfully added to \"%1\"").arg(name);
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

    auto icon = QIcon(":/common/image/notify_success_new.svg");
    DFloatingMessage *pDFloatingMessage = new DFloatingMessage(DFloatingMessage::MessageType::TransientType, m_musicContentWidget);
    pDFloatingMessage->setBlurBackgroundEnabled(true);
    pDFloatingMessage->setMessage(text);
    pDFloatingMessage->setIcon(icon);
    pDFloatingMessage->setDuration(2000);
    DMessageManager::instance()->sendMessage(m_musicContentWidget, pDFloatingMessage);
}

void MainFrame::slotTheme(DGuiApplicationHelper::ColorType themeType)
{
    if (m_musicContentWidget != nullptr) {
        m_musicContentWidget->slotTheme(themeType);
    }
    if (m_footer != nullptr) {
        m_footer->slotTheme(themeType);
    }

    if (m_musicLyricWidget) {
        m_musicLyricWidget->slotTheme(themeType);
    }

    if (m_searchResult) {
        m_searchResult->slotTheme(themeType);
    }
}

void MainFrame::slotSearchEditFoucusIn()
{
    m_titlebarwidget->slotSearchEditFoucusIn();
}

void MainFrame::slotLyricClicked()
{
    // 歌词控件显示与关闭动画
    if (m_musicLyricWidget->isHidden()) {
        m_musicLyricWidget->showAnimation(this->size());
        m_musicContentWidget->showAnimationToUp(this->size() - QSize(0, m_footer->height() + titlebar()->height() + 5));
    } else {
        m_musicLyricWidget->closeAnimation(this->size());
        m_musicContentWidget->closeAnimation(this->size() - QSize(0, m_footer->height() + titlebar()->height() + 5));
    }
}

void MainFrame::slotImportFinished()
{
    // 导入界面显示与关闭动画
    if (m_importWidget->isVisible()) {
        m_musicContentWidget->show();
        m_musicContentWidget->showAnimationToDown(this->size() - QSize(0, m_footer->height() + titlebar()->height() + 5));
        m_footer->show();
        m_importWidget->closeAnimationToDown(this->size());
    }
    m_titlebarwidget->setEnabled(true);
}

void MainFrame::slotShortCutTriggered()
{
    QShortcut *objCut =   dynamic_cast<QShortcut *>(sender()) ;
    Q_ASSERT(objCut);

    if (objCut == addmusicfilesShortcut) {
        m_importWidget->slotImportPathButtonClicked(); //open filedialog
    }

    if (objCut == viewshortcut) {
        QRect rect = window()->geometry();
        QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
        Shortcut sc;
        QStringList shortcutString;
        QString param1 = "-j=" + sc.toStr();
        QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
        shortcutString << "-b" << param1 << param2;

        QProcess *shortcutViewProc = new QProcess(this);
        shortcutViewProc->startDetached("killall deepin-shortcut-viewer");
        shortcutViewProc->startDetached("deepin-shortcut-viewer", shortcutString);

        connect(shortcutViewProc, SIGNAL(finished(int)), shortcutViewProc, SLOT(deleteLater()));
    }

    if (objCut == searchShortcut) {
        m_titlebarwidget->slotSearchEditFoucusIn();
    }

    if (objCut == windowShortcut) {
        if (windowState() == Qt::WindowMaximized) {
            showNormal();
        } else {
            showMaximized();
        }
    }
}

void MainFrame::slotMenuTriggered(QAction *action)
{
    Q_ASSERT(action);

    if (action == m_newSonglistAction) {
        CommonService::getInstance()->addNewSongList();
    }

    if (action == m_addMusicFiles) {
        m_importWidget->slotImportPathButtonClicked();
    }

    if (action == m_equalizer) {
        if (m_dequalizerDialog == nullptr) {
            m_dequalizerDialog = new DequalizerDialog(this);
            connect(m_dequalizerDialog, &DequalizerDialog::setEqualizerEnable,
                    Player::instance(), &Player::setEqualizerEnable);
            connect(m_dequalizerDialog, &DequalizerDialog::setEqualizerpre,
                    Player::instance(), &Player::setEqualizerpre);
            connect(m_dequalizerDialog, &DequalizerDialog::setEqualizerbauds,
                    Player::instance(), &Player::setEqualizerbauds);
            connect(m_dequalizerDialog, &DequalizerDialog::setEqualizerIndex,
                    Player::instance(), &Player::setEqualizerCurMode);
        }
        //配置均衡器参数
        Player::instance()->initEqualizerCfg();
        Dtk::Widget::moveToCenter(m_dequalizerDialog);
        m_dequalizerDialog->exec();
        MusicSettings::sync();
    }

    if (action == m_settings) {
        DSettingsDialog *configDialog = new DSettingsDialog(this);
        AC_SET_OBJECT_NAME(configDialog, AC_configDialog);
        AC_SET_ACCESSIBLE_NAME(configDialog, AC_configDialog);

        configDialog->updateSettings(MusicSettings::settings());
        Dtk::Widget::moveToCenter(configDialog);

        auto curAskCloseAction = MusicSettings::value("base.close.is_close").toBool();
        auto curLastPlaylist = MusicSettings::value("base.play.last_playlist").toString();
        auto curLastMeta = MusicSettings::value("base.play.last_meta").toString();
        auto curLastPosition = MusicSettings::value("base.play.last_position").toInt();

        configDialog->exec();
        delete configDialog;

        MusicSettings::sync();
        MusicSettings::setOption("base.close.is_close", curAskCloseAction);
        MusicSettings::setOption("base.play.last_playlist", curLastPlaylist);
        MusicSettings::setOption("base.play.last_meta", curLastMeta);
        MusicSettings::setOption("base.play.last_position", curLastPosition);

        //update shortcut
        m_footer->updateShortcut();
        //update fade
        Player::instance()->setFadeInOut(MusicSettings::value("base.play.fade_in_out").toBool());
    }
}

void MainFrame::slotSwitchTheme()
{
    MusicSettings::setOption("base.play.theme", DGuiApplicationHelper::instance()->themeType());
}

void MainFrame::slotAllMusicCleared()
{
    qDebug() << "MainFrame::slotAllMusicCleared";
    // 导入界面显示与关闭动画
    m_musicContentWidget->closeAnimationToLeft(this->size() - QSize(0, m_footer->height() + titlebar()->height() + 5));
    m_footer->hide();
    m_importWidget->showImportHint();
    m_importWidget->showAnimationToLeft(this->size());
    m_titlebarwidget->setEnabled(false);
}

void MainFrame::slotAutoPlay(bool bremb)
{
    qDebug() << "slotAutoPlay=========";
    auto lastMeta = MusicSettings::value("base.play.last_meta").toString();
    if (bremb)
        Player::instance()->resume();
    else {
        MediaMeta mt = DataBaseService::getInstance()->getMusicInfoByHash(lastMeta);
        if (!mt.localPath.isEmpty())
            Player::instance()->playMeta(mt);
        else
            qDebug() << __FUNCTION__ << " at line:" << __LINE__ << " localPath is empty.";
    }
}

void MainFrame::slotPlayFromFileMaganager()
{
    qDebug() << "----------openUrl:" << m_firstPlaySong << "all size:" << DataBaseService::getInstance()->allMusicInfosCount();
    //通过路径查询歌曲信息，
    auto localfile = m_firstPlaySong;
    MediaMeta mt = DataBaseService::getInstance()->getMusicInfoByHash(DMusic::filepathHash(m_firstPlaySong));
    if (mt.localPath.isEmpty()) {
        //未导入到数据库
        qCritical() << "fail to start from file manager";
        return;
    }
    qDebug() << "----------playMeta:" << mt.localPath;
    Player::instance()->playMeta(mt);
}

void MainFrame::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    auto geometry = MusicSettings::value("base.play.geometry").toByteArray();

    if (geometry.size() < 4) {
        this->setMinimumSize(QSize(900, 600));
        MusicSettings::setOption("base.play.geometry", saveGeometry());
        geometry = MusicSettings::value("base.play.geometry").toByteArray();
    }
    QDataStream stream(geometry);
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
    qint32 restoredScreenWidth = 0;

    stream >> restoredFrameGeometry
           >> restoredNormalGeometry
           >> restoredScreenNumber
           >> maximized
           >> fullScreen;

    if (majorVersion > 1) {
        stream >> restoredScreenWidth;
    }

    this->resize(QSize(1070, 680));
//    this->show();
    Dtk::Widget::moveToCenter(this);
    if (geometry.isEmpty()) {
        this->resize(QSize(1070, 680));
        Dtk::Widget::moveToCenter(this);
    } else {
        this->restoreGeometry(geometry);
        this->restoreState(MusicSettings::value("base.play.state").toByteArray());
    }
    this->setFocus();
    qDebug() << "zy------MainWindow::showEvent " << QTime::currentTime().toString("hh:mm:ss.zzz");
    if (m_footer) {
        m_footer->setGeometry(0, height() - 85, width(), 80);
    }
    if (m_musicContentWidget) {
        m_musicContentWidget->setGeometry(0, 50, width(), height() - m_footer->height() - titlebar()->height() - 5);
    }
    if (m_importWidget) {
        m_importWidget->setGeometry(0, 50, width(), height() - titlebar()->height());
    }
}

void MainFrame::enterEvent(QEvent *event)
{

}

void MainFrame::resizeEvent(QResizeEvent *e)
{
    DMainWindow::resizeEvent(e);
    QSize newSize = DMainWindow::size();

    m_musicLyricWidget->resize(this->size());

//    if (loadWidget) {
//        loadWidget->setFixedSize(newSize);
//        loadWidget->raise();
//    }

//    if (searchResult) {
//        searchResult->hide();
//    }

    auto titleBarHeight =  m_titlebar->height();
    m_titlebar->raise();
    m_titlebar->resize(newSize.width(), titleBarHeight);

    if (m_importWidget) {
        m_importWidget->setFixedSize(newSize);
    }

//    if (lyricWidget) {
//        lyricWidget->setFixedSize(newSize);
//        musicListWidget->setFixedSize(newSize);
//    }

    if (m_footer) {
        m_footer->setGeometry(0, height() - 85, width(), 80);
    }
    if (m_musicContentWidget) {
        m_musicContentWidget->setGeometry(0, 50, width(), height() - m_footer->height() - titlebar()->height() - 5);
    }

//    if (!d->originCoverImage.isNull()) {
//        d->currentCoverImage = WidgetHelper::cropRect(d->originCoverImage, size());
//    }
}

void MainFrame::closeEvent(QCloseEvent *event)
{
    auto askCloseAction = MusicSettings::value("base.close.close_action").toInt();
    switch (askCloseAction) {
    case 0: {
        MusicSettings::setOption("base.close.is_close", false);
        MusicSettings::setOption("base.play.geometry", saveGeometry());
        break;
    }
    case 1: {
        MusicSettings::setOption("base.play.state", int(windowState()));
        MusicSettings::setOption("base.play.geometry", saveGeometry());
        MusicSettings::setOption("base.close.is_close", true);
        qApp->quit();
        break;
    }
    case 2: {
        CloseConfirmDialog ccd(this);

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
            qApp->quit();
        } else {
            MusicSettings::setOption("base.close.is_close", false);
        }
        break;
    }
    default:
        break;
    }

    MusicSettings::setOption("base.play.last_position", Player::instance()->position());
    this->setFocus();
    DMainWindow::closeEvent(event);
}

