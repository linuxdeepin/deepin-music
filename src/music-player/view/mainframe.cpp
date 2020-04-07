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
#include <QStackedLayout>
#include <QStandardPaths>
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QSystemTrayIcon>
#include <QTimer>

#include <DUtil>
#include <DWidgetUtil>
#include <DAboutDialog>
#include <DDialog>
#include <DApplication>
#include <DTitlebar>
#include <DImageButton>
#include <DFileDialog>
#include <DHiDPIHelper>

#include "../presenter/presenter.h"
#include "../core/metasearchservice.h"
#include "../core/musicsettings.h"
#include "../core/player.h"
#include "../musicapp.h"

#include "widget/titlebarwidget.h"
#include "widget/infodialog.h"
#include "widget/searchresult.h"
#include "widget/closeconfirmdialog.h"
#include "helper/widgethellper.h"

#include "importwidget.h"
#include "playlistwidget.h"
#include "musiclyricwidget.h"
#include "footerwidget.h"
#include "loadwidget.h"
#include "musiclistwidget.h"
#include "shortcut.h"
#include <DSettingsDialog>
#include <DMessageManager>
#include <QShortcut>
DWIDGET_USE_NAMESPACE

const QString s_PropertyViewname = "viewname";
const QString s_PropertyViewnameLyric = "lyric";
const QString s_PropertyViewnamePlay = "playList";
static const int FooterHeight = 70;
static const int AnimationDelay = 400; //ms
static const int BlurRadius = 25;

using namespace Dtk::Widget;

class MainFramePrivate
{
public:
    MainFramePrivate(MainFrame *parent) : q_ptr(parent) {}

    void initUI(bool showLoading);
    void postInitUI();
    void initMenu();
    void setTheme(int type);
    void setPlayListVisible(bool visible);
    void toggleLyricView();
    void togglePlaylist();
    void slideToImportView();
    void showLyricView();
    void hideLyricView();
    void showPlaylistView();
    void hidePlaylistView();
    void resiveistView();
    void slideToMusicListView(bool keepPlaylist);
    void disableControl(int delay = 350);
    void updateSize(QSize newSize);
    void updateViewname(const QString &vm);
    void updateTitlebarViewname(const QString &vm);
    void overrideTitlebarStyle();
    const QString getLastImportPath() const;
    void startTimer();

    //! ui: show info dialog
    void showInfoDialog(const MetaPtr meta);

    DWidget             *centralWidget          = nullptr;
    QStackedLayout      *contentLayout          = nullptr;
    DTitlebar           *titlebar               = nullptr;
    SearchResult        *searchResult           = nullptr;
    TitlebarWidget      *titlebarwidget         = nullptr;
    ImportWidget        *importWidget           = nullptr;
    LoadWidget          *loadWidget             = nullptr;
    PlayListWidget      *playListWidget         = nullptr;
    MUsicLyricWidget    *lyricWidget            = nullptr;
    Footer              *footer                 = nullptr;
    MusicListWidget     *musicListWidget        = nullptr;

    DWidget             *currentWidget          = nullptr;
    InfoDialog          *infoDialog             = nullptr;
//    DSettingsDialog     *configDialog           = nullptr;

    QAction             *newSonglistAction      = nullptr;
    QAction             *colorModeAction        = nullptr;
    QString             coverBackground         = ":/common/image/cover_max.svg";
    QImage              originCoverImage;
    QImage              currentCoverImage;
    QString             viewname                = "";

    QPoint              m_LastMousePos;
    QTimer              *timer                  = nullptr;
    int                 playingCount            = 0;

    QShortcut           *volumeUpShortcut       = nullptr;
    QShortcut           *volumeDownShortcut     = nullptr;
    QShortcut           *nextShortcut           = nullptr;
    QShortcut           *playPauseShortcut      = nullptr;
    QShortcut           *previousShortcut       = nullptr;

    int                 width                   = 0;
    int                 height                  = 0;
    bool                first                   = true;
    MainFrame *q_ptr;
    Q_DECLARE_PUBLIC(MainFrame)
};
void MainFramePrivate::setTheme(int type)
{
    Q_Q(MainFrame);
    if (type == 0) {
        type = DGuiApplicationHelper::instance()->themeType();
    }
    if (musicListWidget != nullptr)
        musicListWidget->slotTheme(type);
    if (playListWidget != nullptr)
        playListWidget->slotTheme(type);
    if (footer != nullptr)
        footer->slotTheme(type);
    if (lyricWidget != nullptr)
        lyricWidget->slotTheme(type);
    if (infoDialog != nullptr)
        infoDialog->setThemeType(type);
    if (importWidget != nullptr)
        importWidget->slotTheme(type);
    if (searchResult != nullptr) {
        searchResult->slotTheme(type);
    }
}
void MainFramePrivate::initMenu()
{
    Q_Q(MainFrame);

    newSonglistAction = new QAction(MainFrame::tr("Add playlist"), q);
    newSonglistAction->setEnabled(false);
    q->connect(newSonglistAction, &QAction::triggered, q, [ = ](bool) {
        Q_EMIT q->addPlaylist(true);
    });

    //    auto addmusic = new QAction(MainFrame::tr("Add folder"), q);
    //    q->connect(addmusic, &QAction::triggered, q, [ = ](bool) {
    //        q->onSelectImportDirectory();
    //    });

    auto addmusicfiles = new QAction(MainFrame::tr("Add music"), q);
    q->connect(addmusicfiles, &QAction::triggered, q, [ = ](bool) {
        q->onSelectImportFiles();
    });

    auto settings = new QAction(MainFrame::tr("Settings"), q);
    q->connect(settings, &QAction::triggered, q, [ = ](bool) {
        DSettingsDialog *configDialog = new DSettingsDialog(q);
        //        configDialog->setProperty("_d_QSSThemename", "dark");
        //        configDialog->setProperty("_d_QSSFilename", "DSettingsDialog");

//        configDialog->setFixedSize(720, 550);
        configDialog->updateSettings(MusicSettings::settings());

        //WidgetHelper::workaround_updateStyle(configDialog, "dlight");
        Dtk::Widget::moveToCenter(configDialog);

        configDialog->exec();
        delete configDialog;
        MusicSettings::sync();

        auto play_pauseStr = MusicSettings::value("shortcuts.all.play_pause").toString();
        if (play_pauseStr.isEmpty())
            playPauseShortcut->setEnabled(false);
        else {
            playPauseShortcut->setEnabled(true);
            playPauseShortcut->setKey(QKeySequence(play_pauseStr));
        }
        auto volume_upStr = MusicSettings::value("shortcuts.all.volume_up").toString();
        if (volume_upStr.isEmpty())
            volumeUpShortcut->setEnabled(false);
        else {
            volumeUpShortcut->setEnabled(true);
            volumeUpShortcut->setKey(QKeySequence(volume_upStr));
        }
        auto volume_downStr = MusicSettings::value("shortcuts.all.volume_down").toString();
        if (volume_downStr.isEmpty())
            volumeDownShortcut->setEnabled(false);
        else {
            volumeDownShortcut->setEnabled(true);
            volumeDownShortcut->setKey(QKeySequence(volume_downStr));
        }
        auto nextStr = MusicSettings::value("shortcuts.all.next").toString();
        if (nextStr.isEmpty())
            nextShortcut->setEnabled(false);
        else {
            nextShortcut->setEnabled(true);
            nextShortcut->setKey(QKeySequence(nextStr));
        }
        auto previousStr = MusicSettings::value("shortcuts.all.previous").toString();
        if (previousStr.isEmpty())
            previousShortcut->setEnabled(false);
        else {
            previousShortcut->setEnabled(true);
            previousShortcut->setKey(QKeySequence(previousStr));
        }
        Q_EMIT q->fadeInOut();
    });

//    bool themeFlag = false;
//    int themeType = MusicSettings::value("base.play.theme").toInt(&themeFlag);
//    if (!themeFlag)
//        themeType = 1;
    int themeType = DGuiApplicationHelper::instance()->themeType();
    colorModeAction = new QAction(MainFrame::tr("Dark theme"), q);
    colorModeAction->setCheckable(true);
    colorModeAction->setChecked(themeType == 2);

    q->connect(colorModeAction, &QAction::triggered, q, [ = ](bool) {
        /*if (DThemeManager::instance()->theme() == "light") {
            colorModeAction->setChecked(true);
            DThemeManager::instance()->setTheme("dark");
        } else {
            colorModeAction->setChecked(false);
            DThemeManager::instance()->setTheme("light");
        }
        MusicSettings::setOption("base.play.theme", DThemeManager::instance()->theme());*/
    });

    QAction *m_close = new QAction(MainFrame::tr("Exit"), q);
    q->connect(m_close, &QAction::triggered, q, [ = ](bool) {
        q->close();
    });

    auto titleMenu = new DMenu(q);
    titleMenu->addAction(newSonglistAction);
//titleMenu->addAction(addmusic);
    titleMenu->addAction(addmusicfiles);
    titleMenu->addSeparator();

//titleMenu->addAction(colorModeAction);
    titleMenu->addAction(settings);
    titleMenu->addSeparator();

    titlebar->setMenu(titleMenu);

//add shortcut
    playPauseShortcut = new QShortcut(q);
    playPauseShortcut->setKey(QKeySequence(MusicSettings::value("shortcuts.all.play_pause").toString()));
    q->connect(playPauseShortcut, &QShortcut::activated, q, [ = ]() {
        //Q_EMIT  q->triggerShortcutAction("shortcuts.all.play_pause");
        footer->onTogglePlayButton();
    });

    volumeUpShortcut = new QShortcut(q);
    volumeUpShortcut->setKey(QKeySequence(MusicSettings::value("shortcuts.all.volume_up").toString()));
    q->connect(volumeUpShortcut, &QShortcut::activated, q, [ = ]() {
        Q_EMIT  q->triggerShortcutAction("shortcuts.all.volume_up");
    });

    volumeDownShortcut = new QShortcut(q);
    volumeDownShortcut->setKey(QKeySequence(MusicSettings::value("shortcuts.all.volume_down").toString()));
    q->connect(volumeDownShortcut, &QShortcut::activated, q, [ = ]() {
        Q_EMIT  q->triggerShortcutAction("shortcuts.all.volume_down");
    });

    nextShortcut = new QShortcut(q);
    nextShortcut->setKey(QKeySequence(MusicSettings::value("shortcuts.all.next").toString()));
    q->connect(nextShortcut, &QShortcut::activated, q, [ = ]() {
        Q_EMIT  q->triggerShortcutAction("shortcuts.all.next");
    });

    previousShortcut = new QShortcut(q);
    previousShortcut->setKey(QKeySequence(MusicSettings::value("shortcuts.all.previous").toString()));
    q->connect(previousShortcut, &QShortcut::activated, q, [ = ]() {
        Q_EMIT  q->triggerShortcutAction("shortcuts.all.previous");
    });

    QShortcut *addmusicfilesShortcut = new QShortcut(q);
    addmusicfilesShortcut->setKey(QKeySequence(QLatin1String("Ctrl+I")));
    q->connect(addmusicfilesShortcut, &QShortcut::activated, q, [ = ]() {
        q->onSelectImportFiles();
    });

    QShortcut *newSonglistShortcut = new QShortcut(q);
    newSonglistShortcut->setKey(QKeySequence(QLatin1String("Ctrl+Shift+N")));
    q->connect(newSonglistShortcut, &QShortcut::activated, q, [ = ]() {
        if (newSonglistAction->isEnabled())
            Q_EMIT q->addPlaylist(true);
    });
}

void MainFramePrivate::initUI(bool showLoading)
{
    showLoading = true;
    Q_Q(MainFrame);
    q->setMinimumSize(QSize(945, 600));
    q->setFocusPolicy(Qt::ClickFocus);

    //titlebar->setBackgroundTransparent(true);
    //overrideTitlebarStyle();

    centralWidget = new QWidget(q);
    contentLayout = new QStackedLayout(centralWidget);
    contentLayout->setContentsMargins(20, 20, 20, 0);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);
    q->setCentralWidget(centralWidget);

    loadWidget = new LoadWidget(q);
    importWidget = new ImportWidget(q);

    footer = new Footer(q);
    footer->enableControl(false);

    contentLayout->addWidget(titlebar);
    contentLayout->addWidget(loadWidget);
    contentLayout->addWidget(importWidget);
    contentLayout->addWidget(footer);

    if (showLoading) {
        loadWidget->show();
        importWidget->hide();
    } else {
        loadWidget->hide();
        importWidget->show();
    }
    footer->hide();

    infoDialog = new InfoDialog(q);

    int themeType = DGuiApplicationHelper::instance()->themeType();
    infoDialog->setThemeType(themeType);
    infoDialog->hide();
#if 0
    footer->show();
#endif
}

void MainFramePrivate::postInitUI()
{
    Q_Q(MainFrame);

    loadWidget->hide();
    newSonglistAction->setDisabled(true);

    playListWidget = footer->getPlayListWidget();

    lyricWidget = new MUsicLyricWidget;
    lyricWidget->setContentsMargins(0, titlebar->height(), 0, FooterHeight + 10);

    musicListWidget = new MusicListWidget;
    musicListWidget->setContentsMargins(0, titlebar->height(), 0, FooterHeight + 10);

    contentLayout->setContentsMargins(0, 0, 0, 0);

    contentLayout->addWidget(importWidget);
    contentLayout->addWidget(lyricWidget);
    contentLayout->addWidget(musicListWidget);

    timer = new QTimer(q);
    q->connect(timer, SIGNAL(timeout()), q, SLOT(changePicture()));

    titlebarwidget->setSearchEnable(false);
    importWidget->show();
    footer->hide();
    footer->setFocus();
    updateSize(q->size());
    infoDialog->move(q->pos().x() + q->size().width() / 2 - infoDialog->width() / 2, q->pos().y() + titlebar->height());
}

void MainFramePrivate::showLyricView()
{
    footer->setPlaylistButtonChecked(false);
    hidePlaylistView();
    auto current = currentWidget ? currentWidget : playListWidget;
    lyricWidget->setFixedSize(current->size());
    WidgetHelper::slideBottom2TopWidget(
        current,  lyricWidget, AnimationDelay);
    currentWidget = lyricWidget;
    titlebar->raise();
    footer->setLyricButtonChecked(true);
    footer->raise();

    updateViewname(s_PropertyViewnameLyric);
}

void MainFramePrivate::hideLyricView()
{
    footer->setPlaylistButtonChecked(false);
    auto current = currentWidget ? currentWidget : playListWidget;
    lyricWidget->setFixedSize(current->size());
    WidgetHelper::slideTop2BottomWidget(
        current, musicListWidget, AnimationDelay);
    titlebar->raise();
    footer->setLyricButtonChecked(false);
    footer->raise();

    updateViewname(s_PropertyViewnameLyric);
}

void MainFramePrivate::showPlaylistView()
{
    if (footer->height() > 80) {
        return;
    }
    QRect start ( 5,  height - 86,
                  width - 10, 80);
    QRect end ( 5,  height - 429,
                width - 10, 423);
    QRect start1 ( 0, 0,
                   width - 10, 0);
    QRect end1 ( 0,  0,
                 width - 10, 314);
    WidgetHelper::slideEdgeWidget2(
        playListWidget, start1, end1, AnimationDelay, true);
    WidgetHelper::slideEdgeWidget(
        footer, playListWidget, start, end, AnimationDelay, true);
    titlebar->raise();
    footer->raise();
    footer->setPlaylistButtonChecked(true);

}

void MainFramePrivate::hidePlaylistView()
{
    if (footer->height() <= 80) {
        return;
    }
    QRect start ( 5,  height - 429,
                  width - 10, 423);
    QRect end ( 5,  height - 86,
                width - 10, 80);

    QRect start1 ( 0,  0,
                   width - 10, 314);
    QRect end1 ( 0, 0,
                 width - 10, 0);
    WidgetHelper::slideEdgeWidget2(
        playListWidget, start1, end1, AnimationDelay, false);
    WidgetHelper::slideEdgeWidget(
        footer, playListWidget, start, end, AnimationDelay, false);
    titlebar->raise();
    footer->setPlaylistButtonChecked(false);
    footer->raise();
}

void MainFramePrivate::resiveistView()
{
    if (first) {
        first = false;
        return ;
    }
    if (playListWidget->isVisible()) {
        QRect start1 ( 0, 0,
                       width - 10, 314);
        QRect end1 ( 0,  0,
                     width - 10, 314);
        WidgetHelper::slideEdgeWidget2(
            playListWidget, start1, end1, AnimationDelay, true);
        QRect rect ( 5,  height - 429,
                     width - 10, 423);
        WidgetHelper::slideEdgeWidget(
            footer, playListWidget, rect, rect, 10, true);
    } else {
        QRect start1 ( 0,  0,
                       width - 10, 0);
        QRect end1 ( 0, 0,
                     width - 10, 0);
        WidgetHelper::slideEdgeWidget2(
            playListWidget, start1, end1, AnimationDelay, false);
        QRect rect ( 5,  height - 86,
                     width - 10, 80);
        WidgetHelper::slideEdgeWidget(
            footer, playListWidget, rect, rect, 10, false);
    }
}


void MainFramePrivate:: slideToImportView()
{
    //    Q_Q(MainFrame);

    titlebarwidget->setSearchEnable(false);
    newSonglistAction->setDisabled(true);
    footer->setLyricButtonChecked(false);
    footer->setPlaylistButtonChecked(false);
    if (importWidget->isVisible()) {
        importWidget->showImportHint();
        footer->enableControl(false);
        return;
    }

    //setPlayListVisible(false);
    playListWidget->hide();
    auto current = currentWidget ? currentWidget : playListWidget;
    importWidget->showImportHint();
    footer->enableControl(false);
    importWidget->setFixedSize(current->size());

    qDebug() << "show importWidget" << current << importWidget;

    WidgetHelper::slideRight2LeftWidget(
        current, importWidget, AnimationDelay);
    footer->enableControl(false);
    currentWidget = importWidget;
    titlebar->raise();
    footer->hide();

    updateViewname("");
}

void MainFramePrivate:: slideToMusicListView(bool keepPlaylist)
{
    Q_Q(MainFrame);

    titlebarwidget->setSearchEnable(true);
    newSonglistAction->setDisabled(false);
    footer->setLyricButtonChecked(false);
    footer->setPlaylistButtonChecked(false);
    auto current = currentWidget ? currentWidget : importWidget;
    if (musicListWidget->isVisible()) {
        musicListWidget->raise();
        titlebar->raise();
        footer->raise();
        return;
    }
    musicListWidget->setFixedSize(current->size());
    WidgetHelper::slideTop2BottomWidget(
        current, musicListWidget, AnimationDelay);
    q->update();
    //disableControl(AnimationDelay);
    currentWidget = musicListWidget;
    titlebar->raise();
    footer->show();
    footer->raise();

    footer->setFocus();
    updateViewname("");
}

void MainFramePrivate::toggleLyricView()
{
    musicListWidget->hide();
    if (lyricWidget->isVisible()) {
        hideLyricView();
        titlebarwidget->setSearchEnable(true);
    } else {
        titlebarwidget->setSearchEnable(false);
        showLyricView();
    }
}

void MainFramePrivate::togglePlaylist()
{

    importWidget->hide();

    if (playListWidget->isVisible()) {
        hidePlaylistView();
        titlebarwidget->setSearchEnable(true);
    } else {
        showPlaylistView();
        titlebarwidget->setSearchEnable(true);
    }
}

void MainFramePrivate::setPlayListVisible(bool visible)
{
    Q_Q(MainFrame);
    footer->setPlaylistButtonChecked(visible);
    if (footer->getShowPlayListFlag() == visible) {
        return;
    }

    footer->showPlayListWidget(q->width(), q->height(), false);
//    int delay = AnimationDelay * 6 / 10;
    //disableControl(delay);
    titlebar->raise();
    footer->raise();

//    QTimer::singleShot(delay * 1, q, [ = ]() {
//        playListWidget->setProperty("moving", false);
//    });
}

void MainFramePrivate::disableControl(int delay)
{
    Q_Q(MainFrame);
    footer->enableControl(false);
    QTimer::singleShot(delay, q, [ = ]() {
        footer->show();
        footer->enableControl(true);
        playListWidget->setEnabled(true);
    });
}

void MainFramePrivate::updateSize(QSize newSize)
{
    if (loadWidget) {
        loadWidget->setFixedSize(newSize);
        loadWidget->raise();
    }

    if (searchResult) {
        searchResult->hide();
    }

    auto titleBarHeight =  titlebar->height();
    titlebar->raise();
    //titlebar->move(0, 0);
    //titlebar->resize(newSize.width(), titleBarHeight);
    titlebar->resize(newSize.width(), titleBarHeight);
    //titlebarwidget->setFixedSize(newSize.width() - titlebar->buttonAreaWidth() - FooterHeight, titleBarHeight);

    importWidget->setFixedSize(newSize);

    if (lyricWidget) {
        lyricWidget->setFixedSize(newSize);
        musicListWidget->setFixedSize(newSize);
    }

    footer->raise();
    width  =  newSize.width();
    height =  newSize.height();
    footer->showPlayListWidget(newSize.width(), newSize.height());
    if (playListWidget != nullptr) {
        resiveistView();
    }
}

void MainFramePrivate::updateViewname(const QString &vm)
{
    Q_Q(MainFrame);
    if (vm == "lyric") {
        footer->setViewname(vm);
    }
    DUtil::TimerSingleShot(AnimationDelay / 2, [this, vm]() {
        updateTitlebarViewname(vm);
    });
    DUtil::TimerSingleShot(AnimationDelay * (q->height() - footer->height() + 24) / q->height(), [this, vm]() {
        footer->setViewname(vm);
    });
}

void MainFramePrivate::updateTitlebarViewname(const QString &vm)
{
    Q_Q(MainFrame);

    q->setProperty("viewname", vm);
    titlebar->setProperty("viewname", vm);
    titlebarwidget->setViewname(vm);
    QStringList objNames;
    objNames  << "DTitlebarDWindowMinButton"
              << "DTitlebarDWindowMaxButton"
              << "DTitlebarDWindowCloseButton"
              << "DTitlebarDWindowOptionButton";

    for (auto &objname : objNames) {
        auto titlebarBt = q->titlebar()->findChild<QWidget *>(objname);
        if (!titlebarBt) {
            continue;
        }
        titlebarBt->setProperty("viewname", vm);
    }
}

void MainFramePrivate::overrideTitlebarStyle()
{
    titlebar->setObjectName("Titlebar");

    QStringList objNames;
    objNames  << "DTitlebarDWindowMinButton"
              << "DTitlebarDWindowMaxButton"
              << "DTitlebarDWindowCloseButton"
              << "DTitlebarDWindowOptionButton";

    for (auto &objname : objNames) {
        auto titlebarBt = titlebar->findChild<QWidget *>(objname);
        if (!titlebarBt) {
            continue;
        }
    }
}

const QString MainFramePrivate::getLastImportPath() const
{
    QString lastImportPath = MusicSettings::value("base.play.last_import_path").toString();

    QDir lastImportDir = QDir(lastImportPath);
    if (!lastImportDir.exists() || lastImportPath.isEmpty()) {
        lastImportPath =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    } else {
        // blumia: always use the path from QDir, QDir also accept relative path string and it will pass
        //         the `QDir::exists()` checking
        lastImportPath = lastImportDir.absolutePath();
    }

    return lastImportPath;
}

void MainFramePrivate::startTimer()
{
    timer->start(250);
}

void MainFramePrivate::showInfoDialog(const MetaPtr meta)
{
    infoDialog->show();
    infoDialog->updateInfo(meta);
}

MainFrame::MainFrame(QWidget *parent) :
    DMainWindow(parent), dd_ptr(new MainFramePrivate(this))
{
    setObjectName("MainFrame");

    QString descriptionText = MainFrame::tr("Music is a local music player with beautiful design and simple functions.");
    QString acknowledgementLink = "https://www.deepin.org/acknowledgments/deepin-music#thanks";
    qApp->setProductName(QApplication::tr("Music"));
    qApp->setApplicationAcknowledgementPage(acknowledgementLink);
    qApp->setProductIcon(QIcon::fromTheme("deepin-music"));
    qApp->setApplicationDescription(descriptionText);

    this->setWindowTitle(tr("Music"));

    Q_D(MainFrame);
    d->titlebarwidget = new TitlebarWidget(this);
    //d->titlebarwidget->setFixedSize(365, 50);
    //d->titlebarwidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->searchResult = new SearchResult(this);
    d->searchResult->show();
    d->titlebarwidget->setResultWidget(d->searchResult);
    d->titlebarwidget->setEnabled(false);
    d->titlebarwidget->show();

    d->titlebar = titlebar();
    d->titlebar->setFixedHeight(50);
    d->titlebar->setTitle(tr("Music"));
    d->titlebar->setIcon(QIcon::fromTheme("deepin-music"));    //titlebar->setCustomWidget(titlebarwidget, Qt::AlignLeft, false);
    //    d->titlebar->setTitle(tr("Music"));
    d->titlebar->setCustomWidget(d->titlebarwidget, true);
    d->titlebar->resize(width(), 50);
    QShortcut *viewshortcut = new QShortcut(this);
    viewshortcut->setKey(QKeySequence(QLatin1String("Ctrl+Shift+/")));
    connect(viewshortcut, SIGNAL(activated()), this, SLOT(onViewShortcut()));

    QShortcut *searchShortcut = new QShortcut(this);
    searchShortcut->setKey(QKeySequence(QLatin1String("Ctrl+F")));
    connect(searchShortcut, &QShortcut::activated, this, [ = ]() {
        d->titlebarwidget->setEditStatus();
    });

    QShortcut *windowShortcut = new QShortcut(this);
    windowShortcut->setKey(QKeySequence(QLatin1String("Ctrl+Alt+F")));
    connect(windowShortcut, &QShortcut::activated, this, [ = ]() {
        if (windowState() == Qt::WindowMaximized) {
            showNormal();
        } else {
            showMaximized();
        }
    });
}

MainFrame::~MainFrame()
{
    MusicSettings::setOption("base.play.state", int(windowState()));
    MusicSettings::setOption("base.play.geometry", saveGeometry());
}

void MainFrame::initUI(bool showLoading)
{
    Q_D(MainFrame);

    d->initUI(showLoading);

    d->initMenu();
}

void MainFrame::postInitUI()
{
    Q_D(MainFrame);

    d->postInitUI();
    updateUI();
    focusPlayList();

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
        Q_EMIT  triggerShortcutAction("shortcuts.all.play_pause");
    });
    connect(prevAction, &QAction::triggered,
    this, [ = ]() {
        Q_EMIT  triggerShortcutAction("shortcuts.all.previous");
    });
    connect(nextAction, &QAction::triggered,
    this, [ = ]() {
        Q_EMIT  triggerShortcutAction("shortcuts.all.next");
    });
    connect(quitAction, &QAction::triggered,
    this, [ = ]() {
        Q_EMIT  requitQuit();
    });
    connect(trayIcon, &QSystemTrayIcon::activated,
    this, [ = ](QSystemTrayIcon::ActivationReason reason) {
        if (QSystemTrayIcon::Trigger == reason) {
            if (isVisible()) {
                if (isMinimized()) {
                    showNormal();
                    // when window flags changed, should call hide() and show()
                    hide();
                    show();
                } else {
                    showMinimized();
                }
            } else {
                showNormal();
            }
        }
    });
}

void MainFrame::binding(Presenter *presenter)
{
    Q_D(MainFrame);

    d->playListWidget->setCurPlaylist(presenter->playlist(PlayMusicListID));
    d->footer->setCurPlaylist(presenter->playlist(PlayMusicListID));

    connect(this, &MainFrame::importSelectFiles, presenter, &Presenter::onImportFiles);
    connect(this, &MainFrame::addPlaylist, presenter, &Presenter::onPlaylistAdd);
    connect(this, &MainFrame::fadeInOut, presenter, &Presenter::onFadeInOut);

    connect(presenter, &Presenter::searchResult,
    d->musicListWidget, [ = ](QString searchText, QList<PlaylistPtr> resultlist) {
        Q_EMIT  d->musicListWidget->seaResult(searchText, resultlist);
    });

    //    connect(d->titlebar, &Titlebar::mouseMoving, this, &MainFrame::moveWindow);
    //    connect(d->footer, &Footer::mouseMoving, this, &MainFrame::moveWindow);

#ifdef Q_OS_WIN
    connect(d->titlebar, &Titlebar::mousePosMoving,
    this, [ = ](Qt::MouseButton /*button*/, QPoint pos) {
        move(pos - d->m_LastMousePos);
    });

    connect(d->titlebar, &Titlebar::mousePosPressed,
    this, [ = ](Qt::MouseButtons /*button*/, QPoint pos) {
        // TODO: fix margin
        pos.setY(pos.y() - 10);
        d->m_LastMousePos = pos - this->mapToParent(this->pos());
    });
#endif

    //    connect(d->titlebarwidget, &TitlebarWidget::locateMusicInAllMusiclist,
    //            presenter, &Presenter::onLocateMusicAtAll);
    //    connect(d->titlebarwidget, &TitlebarWidget::search,
    //            presenter, &Presenter::onSearchText);
    connect(d->titlebarwidget, &TitlebarWidget::searchExited,
            presenter, &Presenter::onExitSearch);
    connect(d->titlebarwidget, &TitlebarWidget::searchText,
            presenter, &Presenter::onSearchText);

    connect(d->titlebarwidget, &TitlebarWidget::searchExited,
    presenter, [ = ]() {
        Q_EMIT  d->musicListWidget->closeSearch();
    });

    //    connect(d->titlebarwidget, &TitlebarWidget::searchExited,
    //            d->musicListWidget, &MusicListWidget::onSearchText);

    connect(d->importWidget, &ImportWidget::scanMusicDirectory,
            presenter, &Presenter::onScanMusicDirectory);
    connect(d->importWidget, &ImportWidget::selectImportDirectory,
            this, &MainFrame::onSelectImportDirectory);
    connect(d->importWidget, &ImportWidget::importFiles,
            this, &MainFrame::onSelectImportFiles);
    connect(d->importWidget, &ImportWidget::importSelectFiles,
    this, [ = ](const QStringList & urllist) {
        d->importWidget->showWaitHint();
        Q_EMIT importSelectFiles(urllist, nullptr);
    });

    connect(presenter, &Presenter::notifyAddToPlaylist,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList metaPtrList, int count) {
        auto icon = QIcon(":/common/image/notify_success_new.svg");
        QFontMetrics fm(font());
        auto displayName = fm.elidedText(playlist->displayName(), Qt::ElideMiddle, 300);
        //        auto text = tr("Added to %1").arg(displayName);
        //        if (playlist->id() == FavMusicListID)
        //            text = tr("Successfully added to \"%1\"").arg(displayName);
        auto text = tr("Successfully added to \"%1\"").arg(displayName);
        int curCount = metaPtrList.size() - count;
        if (count > 0) {
            if (metaPtrList.size() == 1 || curCount == 0)
                text = tr("Already added to the playlist");
            else {
                if (curCount == 1)
                    text = tr("1 song added");
                else
                    text = tr("%1 songs added").arg(curCount);
            }
        }
        //DMessageManager::instance()->sendMessage(d->footer, icon, text);
        if (playlist->id() != AlbumMusicListID && playlist->id() != ArtistMusicListID
                && playlist->id() != PlayMusicListID)
            this->sendMessage(icon, text);

        QWidget *content = this->findChild<QWidget *>("_d_message_manager_content");
        if (nullptr != content)
            content->setContentsMargins(0, 0, 0, 90);
    });

    //    connect(presenter, &Presenter::showPlaylist,
    //    this, [ = ](bool show) {
    //        d->setPlaylistVisible(show);
    //    });

    connect(presenter, &Presenter::showMusicList,
    this, [ = ](PlaylistPtr playlist) {
        auto current = d->currentWidget ? d->currentWidget : d->importWidget;
        //d->playListWidget->resize(current->size());
        //d->playListWidget->show();
        d->musicListWidget->resize(current->size());
        d->musicListWidget->show();
        d->currentWidget = d->musicListWidget;
        d->importWidget->hide();
        d->playListWidget->onMusiclistChanged(playlist);
        d->musicListWidget->onMusiclistChanged(playlist);
        d->disableControl(false);
        d->titlebarwidget->setSearchEnable(true);
        d->newSonglistAction->setEnabled(true);
    });

    connect(presenter, &Presenter::coverSearchFinished,
    this, [ = ](const MetaPtr, const DMusic::SearchMeta &, const QByteArray & coverData) {
        if (coverData.length() < 32) {
            setCoverBackground(coverBackground());
            this->update();
            return;
        }
        QImage image = QImage::fromData(coverData);
        if (image.isNull()) {
            setCoverBackground(coverBackground());
            this->update();
            return;
        }

        d->originCoverImage = WidgetHelper::blurImage(image, BlurRadius).toImage();
        d->currentCoverImage = WidgetHelper::cropRect(d->originCoverImage, size());
        update();
    });

    connect(presenter, &Presenter::musicStoped,
    this, [ = ](PlaylistPtr, const MetaPtr) {
        setCoverBackground(coverBackground());
        d->timer->stop();
    });

    connect(presenter, &Presenter::notifyMusciError,
    this, [ = ](PlaylistPtr playlist, const MetaPtr  meta, int /*error*/) {
        Dtk::Widget::DDialog warnDlg(this);
        warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
        warnDlg.setTextFormat(Qt::RichText);
        warnDlg.setTitle(tr("File is invalid or does not exist, load failed"));
        warnDlg.addButtons(QStringList() << tr("OK"));
        warnDlg.setDefaultButton(0);

        if (0 == warnDlg.exec()) {
            auto curPlaylist = d->playListWidget->curPlaylist();
            if (curPlaylist->canNext()) {
                bool existFlag = false;
                for (auto curMeta : curPlaylist->allmusic()) {
                    if (!curMeta->invalid || QFile::exists(curMeta->localPath)) {
                        Q_EMIT presenter->playNext(curPlaylist, meta);
                        existFlag = true;
                        break;
                    }
                }
                if (!existFlag) {
                    d->timer->stop();
                }
            } else {
                d->timer->stop();
            }
        } else {
            d->timer->stop();
        }
    });

    connect(presenter, &Presenter::metaLibraryClean,
    this, [ = ]() {
        d->slideToImportView();
        d->titlebarwidget->clearSearch();
        d->footer->onMediaLibraryClean();
    });

    connect(presenter, &Presenter::scanFinished,
    this, [ = ](const QString & /*jobid*/, int mediaCount) {
        if (0 == mediaCount) {
            QString message = QString(tr("Import failed, no valid music file found"));
            Dtk::Widget::DDialog warnDlg(this);
            warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
            warnDlg.setTextFormat(Qt::AutoText);
            warnDlg.setTitle(message);
            warnDlg.addButtons(QStringList() << tr("OK"));
            warnDlg.setDefaultButton(0);
            if (0 == warnDlg.exec()) {
                return;
            }
        }
    });

    connect(presenter, &Presenter::musicPlayed,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        Q_ASSERT(!playlist.isNull());
        Q_ASSERT(!meta.isNull());

        qApp->setApplicationDisplayName(playlist->displayName());
        this->setWindowTitle(meta->title);
        d->startTimer();
    });

    connect(presenter, &Presenter::musicPaused,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        d->timer->stop();
    });

    connect(presenter, &Presenter::meidaFilesImported,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList metalist) {
        d->setPlayListVisible(false);
        d->playListWidget->onMusiclistChanged(playlist);
        d->musicListWidget->onMusicListAdded(playlist, metalist);
        d->disableControl(true);
        d->slideToMusicListView(true);
    });

    connect(presenter, &Presenter::requestImportFiles,
    this, [ = ]() {
        onSelectImportDirectory();
    });

    connect(presenter, &Presenter::currentMusicListChanged,
    this, [ = ](PlaylistPtr playlist) {
        if (playlist && playlist->id() != SearchMusicListID) {
            d->titlebarwidget->exitSearch();
        }
        if (d->lyricWidget->isVisible()) {
            d->setPlayListVisible(true);
        }
    });

    connect(presenter, &Presenter::requestMusicListMenu,
    this, [ = ](const QPoint & pos, PlaylistPtr selectedlist, PlaylistPtr favlist, QList<PlaylistPtr >newlists, char type) {
        if (type > 0) {
            d->musicListWidget->onCustomContextMenuRequest(pos, selectedlist, favlist, newlists, type);
        } else {
            d->playListWidget->onCustomContextMenuRequest(pos, selectedlist, favlist, newlists);
        }
    });

    connect(d->footer,  &Footer::locateMusic,
    this, [ = ](PlaylistPtr playlist, const MetaPtr) {
        if (playlist && playlist->id() != SearchMusicListID) {
            d->titlebarwidget->exitSearch();
        }
        if (d->lyricWidget->isVisible()) {
            d->setPlayListVisible(true);
        }
    });

    //搜索框
    connect(d->titlebarwidget, &TitlebarWidget::searchCand,
            presenter, &Presenter::onSearchCand);

    connect(presenter, &Presenter::searchCand,
            d->searchResult, &SearchResult::onSearchCand, Qt::QueuedConnection);

    // PlayList
    connect(d->playListWidget, &PlayListWidget::showInfoDialog,
    this, [ = ](const MetaPtr meta) {
        d->showInfoDialog(meta);
    });

    connect(d->playListWidget, &PlayListWidget::playall,
            presenter, &Presenter::onPlayall);

    connect(d->playListWidget, &PlayListWidget::resort,
            presenter, &Presenter::onResort);

    connect(d->playListWidget, &PlayListWidget::playMedia,
            presenter, &Presenter::onSyncMusicPlay);
    connect(d->playListWidget,  &PlayListWidget::resume,
            presenter, &Presenter::onSyncMusicResume);
    connect(d->playListWidget, &PlayListWidget::updateMetaCodec,
            presenter, &Presenter::onUpdateMetaCodec);
    connect(d->playListWidget, &PlayListWidget::requestCustomContextMenu,
            presenter, &Presenter::onRequestMusiclistMenu);
    connect(d->playListWidget, &PlayListWidget::addToPlaylist,
            presenter, &Presenter::onAddToPlaylist);
    connect(d->playListWidget, &PlayListWidget::musiclistRemove,
            presenter, &Presenter::onMusiclistRemove);
    connect(d->playListWidget, &PlayListWidget::musiclistDelete,
            presenter, &Presenter::onMusiclistDelete);
    connect(d->playListWidget, &PlayListWidget::addMetasFavourite,
            presenter, &Presenter::onAddMetasFavourite);
    connect(d->playListWidget, &PlayListWidget::removeMetasFavourite,
            presenter, &Presenter::onRemoveMetasFavourite);
    connect(d->playListWidget,  &PlayListWidget::pause,
            presenter, &Presenter::onMusicPause);
    connect(d->playListWidget,  &PlayListWidget::musicFileMiss,
            presenter, &Presenter::musicFileMiss);

    connect(d->playListWidget, &PlayListWidget::importSelectFiles,
    this, [ = ](PlaylistPtr playlist, QStringList urllist) {
        presenter->requestImportPaths(playlist, urllist);
    });
    connect(d->playListWidget, &PlayListWidget::updateMetaCodec,
            d->footer, &Footer::onUpdateMetaCodec);

    connect(presenter, &Presenter::musicListResorted,
            d->playListWidget, &PlayListWidget::onMusiclistChanged);

    connect(presenter, &Presenter::currentMusicListChanged,
            d->playListWidget,  &PlayListWidget::onMusiclistChanged);
    connect(presenter, &Presenter::musicPlayed,
            d->playListWidget,  &PlayListWidget::onMusicPlayed);
    connect(presenter, &Presenter::musicError,
            d->playListWidget,  &PlayListWidget::onMusicError);
    connect(presenter, &Presenter::musicListAdded,
            d->playListWidget,  &PlayListWidget::onMusicListAdded);
    connect(presenter, &Presenter::musicListRemoved,
            d->playListWidget,  &PlayListWidget::onMusicListRemoved);
    connect(presenter, &Presenter::locateMusic,
            d->playListWidget,  &PlayListWidget::onLocate);

    connect(presenter, &Presenter::progrossChanged,
            d->lyricWidget, &MUsicLyricWidget::onProgressChanged);
    connect(presenter, &Presenter::musicPlayed,
            d->lyricWidget, &MUsicLyricWidget::onMusicPlayed);
    connect(presenter, &Presenter::coverSearchFinished,
            d->lyricWidget, &MUsicLyricWidget::onCoverChanged);
    connect(presenter, &Presenter::lyricSearchFinished,
            d->lyricWidget, &MUsicLyricWidget::onLyricChanged);
    connect(presenter, &Presenter::contextSearchFinished,
            d->lyricWidget, &MUsicLyricWidget::onContextSearchFinished);
    connect(presenter, &Presenter::musicStoped,
            d->lyricWidget,  &MUsicLyricWidget::onMusicStop);

    connect(d->lyricWidget,  &MUsicLyricWidget::requestContextSearch,
            presenter, &Presenter::requestContextSearch);
    connect(d->lyricWidget, &MUsicLyricWidget::changeMetaCache,
            presenter, &Presenter::onChangeSearchMetaCache);


    // footer
    connect(d->footer, &Footer::toggleLyricView,
    this, [ = ]() {
        d->toggleLyricView();
    });
    connect(d->footer, &Footer::togglePlaylist,
    this, [ = ]() {
        d->togglePlaylist();
    });


    connect(d->footer,  &Footer::locateMusic,
            d->playListWidget, &PlayListWidget::onLocate);
    connect(d->footer,  &Footer::play,
            presenter, &Presenter::onSyncMusicPlay);
    connect(d->footer,  &Footer::resume,
            presenter, &Presenter::onSyncMusicResume);
    connect(d->footer,  &Footer::pause,
            presenter, &Presenter::onMusicPause);
    connect(d->footer,  &Footer::next,
            presenter, &Presenter::onSyncMusicNext);
    connect(d->footer,  &Footer::prev,
            presenter, &Presenter::onSyncMusicPrev);
    connect(d->footer,  &Footer::changeProgress,
            presenter, &Presenter::onChangeProgress);
    connect(d->footer,  &Footer::volumeChanged,
            presenter, &Presenter::onVolumeChanged);
    connect(d->footer,  &Footer::toggleMute,
            presenter, &Presenter::onToggleMute);
    connect(d->footer,  &Footer::modeChanged,
            presenter, &Presenter::onPlayModeChanged);
    connect(d->footer,  &Footer::toggleFavourite,
            presenter, &Presenter::onToggleFavourite);
    connect(d->footer, &Footer::pause,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        d->timer->stop();
    });

    connect(d->footer,  &Footer::progressRealHeightChanged,
    this, [ = ](qreal realHeight) {
        int margin = FooterHeight
                     + static_cast<int>(realHeight);
        d->playListWidget->setContentsMargins(0, 0, 0, margin);
    });

    connect(presenter, &Presenter::modeChanged,
            d->footer,  &Footer::onModeChange);
    connect(presenter, &Presenter::musicListAdded,
            d->footer,  &Footer::onMusicListAdded);
    connect(presenter, &Presenter::musicListRemoved,
            d->footer,  &Footer::onMusicListRemoved);
    connect(presenter, &Presenter::musicPlayed,
            d->footer,  &Footer::onMusicPlayed);
    connect(presenter, &Presenter::musicPaused,
            d->footer,  &Footer::onMusicPause);
    connect(presenter, &Presenter::musicStoped,
            d->footer,  &Footer::onMusicStoped);
    connect(presenter, &Presenter::progrossChanged,
            d->footer,  &Footer::onProgressChanged);
    connect(presenter, &Presenter::coverSearchFinished,
            d->footer,  &Footer::onCoverChanged);
    connect(presenter, &Presenter::volumeChanged,
            d->footer,  &Footer::onVolumeChanged);
    connect(presenter, &Presenter::mutedChanged,
            d->footer,  &Footer::onMutedChanged);
    connect(presenter, &Presenter::musicError,
            d->footer,  &Footer::onMusicError);
    connect(presenter, &Presenter::audioBufferProbed,
            d->footer,  &Footer::audioBufferProbed);
    connect(presenter, &Presenter::metaBuffer,
            d->footer,  &Footer::metaBuffer);

    // musiclist
    connect(presenter, &Presenter::playlistAdded,
            d->musicListWidget,  &MusicListWidget::onPlaylistAdded);
    connect(presenter, &Presenter::musicPlayed,
            d->musicListWidget,  &MusicListWidget::onMusicPlayed);
    connect(presenter, &Presenter::currentMusicListChanged,
            d->musicListWidget,  &MusicListWidget::onCurrentChanged);

    connect(presenter, &Presenter::musicListResorted,
            d->musicListWidget, &MusicListWidget::onMusiclistChanged);

    connect(presenter, &Presenter::musicListAdded,
            d->musicListWidget,  &MusicListWidget::onMusicListAdded);
    connect(presenter, &Presenter::musicListRemoved,
            d->musicListWidget,  &MusicListWidget::onMusicListRemoved);

    connect(d->musicListWidget, &MusicListWidget::requestCustomContextMenu,
            presenter, &Presenter::onRequestMusiclistMenu);

    connect(d->musicListWidget, &MusicListWidget::showInfoDialog,
    this, [ = ](const MetaPtr meta) {
        d->showInfoDialog(meta);
    });

    connect(d->musicListWidget,  &MusicListWidget::modeChanged,
            presenter, &Presenter::onPlayModeChanged);

    connect(d->musicListWidget, &MusicListWidget::resort,
            presenter, &Presenter::onResort);

    connect(d->musicListWidget,  &MusicListWidget::addPlaylist,
            presenter, &Presenter::onPlaylistAdd);
    connect(d->musicListWidget,  &MusicListWidget::selectPlaylist,
            presenter, &Presenter::onCurrentPlaylistChanged);
    connect(d->musicListWidget,  &MusicListWidget::playall,
            presenter, &Presenter::onPlayall);
    connect(d->musicListWidget,  &MusicListWidget::customResort,
            presenter, &Presenter::onCustomResort);
    connect(d->musicListWidget, &MusicListWidget::playMedia,
            presenter, &Presenter::onSyncMusicPlay);
    connect(d->musicListWidget,  &MusicListWidget::resume,
            presenter, &Presenter::onSyncMusicResume);
    connect(d->musicListWidget,  &MusicListWidget::pause,
            presenter, &Presenter::onMusicPause);
    connect(d->musicListWidget, &MusicListWidget::updateMetaCodec,
            presenter, &Presenter::onUpdateMetaCodec);
    connect(d->musicListWidget, &MusicListWidget::addToPlaylist,
            presenter, &Presenter::onAddToPlaylist);
    connect(d->musicListWidget, &MusicListWidget::musiclistRemove,
            presenter, &Presenter::onMusiclistRemove);
    connect(d->musicListWidget, &MusicListWidget::musiclistDelete,
            presenter, &Presenter::onMusiclistDelete);

    connect(presenter, &Presenter::musicListClear,
    this, [ = ]() {
        qApp->setApplicationDisplayName("");
        this->setWindowTitle(tr("Music"));
    });
    connect(d->musicListWidget, &MusicListWidget::importSelectFiles,
    this, [ = ](PlaylistPtr playlist, QStringList urllist) {
        presenter->requestImportPaths(playlist, urllist);
    });
    connect(d->musicListWidget, &MusicListWidget::addMetasFavourite,
            presenter, &Presenter::onAddMetasFavourite);
    connect(d->musicListWidget, &MusicListWidget::removeMetasFavourite,
            presenter, &Presenter::onRemoveMetasFavourite);

    connect(d->musicListWidget, &MusicListWidget::selectedPlaylistChange,
            d->searchResult, &SearchResult::selectPlaylist);
    connect(d->musicListWidget, &MusicListWidget::selectedPlaylistChange,
            d->titlebarwidget, &TitlebarWidget::selectPlaylist);
    //    connect(d->musicListWidget,  &MusicListWidget::hidePlaylist,
    //    this, [ = ]() {
    //        d->setPlaylistVisible(false);
    //    });

    //add Shortcut
    QShortcut *muteShortcut = new QShortcut(this);
    muteShortcut->setKey(QKeySequence(QLatin1String("M")));
    connect(muteShortcut, &QShortcut::activated, presenter, &Presenter::onToggleMute);
}

void MainFrame::focusPlayList()
{
    Q_D(const MainFrame);
    d->playListWidget->setFocus();
}

QString MainFrame::coverBackground() const
{
    Q_D(const MainFrame);
    return d->coverBackground;
}

void MainFrame::updateUI()
{
    Q_D(MainFrame);
    setCoverBackground(coverBackground());
    d->lyricWidget->updateUI();
    d->titlebarwidget->setEnabled(true);
    d->titlebarwidget->show();
}

void MainFrame::setCoverBackground(QString coverBackground)
{
    Q_D(MainFrame);
    d->coverBackground = coverBackground;
    QImage image = QImage(coverBackground);
    d->originCoverImage = WidgetHelper::blurImage(image, BlurRadius).toImage();
    d->currentCoverImage = WidgetHelper::cropRect(d->originCoverImage, size());

    d->lyricWidget->updateUI();
}

void MainFrame::onSelectImportDirectory()
{
    Q_D(const MainFrame);
    DFileDialog fileDlg(this);

    QString lastImportPath = d->getLastImportPath();

    fileDlg.setDirectory(lastImportPath);

    fileDlg.setViewMode(DFileDialog::Detail);
    fileDlg.setFileMode(DFileDialog::Directory);
    if (DFileDialog::Accepted == fileDlg.exec()) {
        d->importWidget->showWaitHint();
        MusicSettings::setOption("base.play.last_import_path",  fileDlg.directory().path());
        Q_EMIT importSelectFiles(fileDlg.selectedFiles(), nullptr);
    }
}

void MainFrame::onSelectImportFiles()
{
    Q_D(const MainFrame);
    DFileDialog fileDlg(this);

    QString lastImportPath = d->getLastImportPath();

    fileDlg.setDirectory(lastImportPath);

    QString selfilter = tr("All music") + (" (%1)");
    selfilter = selfilter.arg(Player::instance()->supportedSuffixList().join(" "));
    fileDlg.setViewMode(DFileDialog::Detail);
    fileDlg.setFileMode(DFileDialog::ExistingFiles);
    fileDlg.setOption(DFileDialog::HideNameFilterDetails);

    fileDlg.setNameFilter(selfilter);
    fileDlg.selectNameFilter(selfilter);
    if (DFileDialog::Accepted == fileDlg.exec()) {
        d->importWidget->showWaitHint();
        MusicSettings::setOption("base.play.last_import_path",  fileDlg.directory().path());
        Q_EMIT importSelectFiles(fileDlg.selectedFiles(), d->musicListWidget->curPlaylist());
    }
}

void MainFrame::slotTheme(int type)
{
    Q_D(MainFrame);

    d->setTheme(type);
}

void MainFrame::changePicture()
{
    Q_D(MainFrame);
    int curCount = d->playingCount % 4;
    d->playingCount = curCount + 1;
    QPixmap pixmap, albumPixmap, sidebarPixmap;
    switch (curCount) {
    case 0:
        pixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music1.svg");
        albumPixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music_white_album_cover/music1.svg");
        sidebarPixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music_withe_sidebar/music1.svg");
        break;
    case 1:
        pixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music2.svg");
        albumPixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music_white_album_cover/music2.svg");
        sidebarPixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music_withe_sidebar/music2.svg");
        break;
    case 2:
        pixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music3.svg");
        albumPixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music_white_album_cover/music3.svg");
        sidebarPixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music_withe_sidebar/music3.svg");
        break;
    case 3:
        pixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music4.svg");
        albumPixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music_white_album_cover/music4.svg");
        sidebarPixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music_withe_sidebar/music4.svg");
        break;
    default:
        pixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music1.svg");
        albumPixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music_white_album_cover/music1.svg");
        sidebarPixmap = DHiDPIHelper::loadNxPixmap(":/mpimage/light/music_withe_sidebar/music1.svg");
        break;
    }
    d->musicListWidget->changePicture(pixmap, albumPixmap, sidebarPixmap);
    d->playListWidget->changePicture(pixmap, sidebarPixmap, albumPixmap);
}

void MainFrame::onViewShortcut()
{
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

bool MainFrame::eventFilter(QObject *obj, QEvent *e)
{
    Q_D(const MainFrame);

    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);

        auto keyModifiers = ke->modifiers();
        auto key = static_cast<Qt::Key>(ke->key());

        QStringList sclist;
        sclist << "shortcuts.all.next"
               << "shortcuts.all.play_pause"
               << "shortcuts.all.previous"
               << "shortcuts.all.volume_down"
               << "shortcuts.all.volume_up";

        for (auto optkey : sclist) {
            auto shortcut = MusicSettings::value(optkey).toStringList();
            auto modifiersstr = shortcut.value(0);
            auto scmodifiers = static_cast<Qt::KeyboardModifier>(modifiersstr.toInt());
            auto keystr = shortcut.value(1);
            auto sckey = static_cast<Qt::Key>(keystr.toInt());

            if (scmodifiers == keyModifiers && key == sckey && !ke->isAutoRepeat()) {
                //qDebug() << "match " << optkey << ke->count() << ke->isAutoRepeat();
                Q_EMIT  triggerShortcutAction(optkey);
                return true;
            }
        }
    }

    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        if (obj->objectName() == this->objectName() || this->objectName() + "Window" == obj->objectName()) {
            QPoint mousePos = me->pos();
            auto geometry = d->footer->geometry().marginsAdded(QMargins(0, 0, 40, 40));
            if (!geometry.contains(mousePos)) {
                DUtil::TimerSingleShot(50, [this]() {
                    //this->d_func()->setPlayListVisible(false);
                    this->d_func()->hidePlaylistView();
                });
            }
        }
    }

    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        if (obj->objectName() == this->objectName() || this->objectName() + "Window" == obj->objectName()) {
            QPoint mousePos = me->pos();
            //            qDebug() << "lyricView checkHiddenSearch" << me->pos() << QCursor::pos() << obj;
            d->lyricWidget->checkHiddenSearch(mousePos);
        }
    }

    return qApp->eventFilter(obj, e);
}
void MainFrame::enterEvent(QEvent *e)
{
    //    setCursor(QCursor(Qt::PointingHandCursor));
    DMainWindow::enterEvent(e);
}

void MainFrame::resizeEvent(QResizeEvent *e)
{
    Q_D(MainFrame);

    DMainWindow::resizeEvent(e);
    QSize newSize = DMainWindow::size();
    d->updateSize(newSize);

    if (!d->originCoverImage.isNull()) {
        d->currentCoverImage = WidgetHelper::cropRect(d->originCoverImage, size());
    }
}

void MainFrame::closeEvent(QCloseEvent *event)
{
    auto askCloseAction = MusicSettings::value("base.close.ask_close_action").toBool();
    if (askCloseAction) {
        CloseConfirmDialog ccd(this);
        // fix close style
        auto titlebarBt = titlebar()->findChild<QWidget *>("DTitlebarDWindowCloseButton");
        auto closeBt = qobject_cast<DImageButton *>(titlebarBt);
        if (closeBt) {
            closeBt->setState(DImageButton::Normal);
        }

        auto clickedButtonIndex = ccd.exec();
        qDebug() << "clickedButtonIndex:" << clickedButtonIndex;
        // 1 is confirm button
        if (1 != clickedButtonIndex) {
            // fix button style
            event->ignore();
            return;
        }
        MusicSettings::setOption("base.close.ask_close_action", !ccd.isRemember());
        MusicSettings::setOption("base.close.close_action", ccd.closeAction());
    }

    auto closeAction = MusicSettings::value("base.close.close_action").toInt();
    if (CloseConfirmDialog::QuitOnClose == closeAction) {
        MusicSettings::setOption("base.play.state", int(windowState()));
        MusicSettings::setOption("base.play.geometry", saveGeometry());
        DMainWindow::closeEvent(event);
    }
    DMainWindow::closeEvent(event);
}

void MainFrame::paintEvent(QPaintEvent *e)
{
    Q_D(MainFrame);
    QPainter p(this);

    //p.drawImage(rect(), d->currentCoverImage);
    DMainWindow::paintEvent(e);
}
