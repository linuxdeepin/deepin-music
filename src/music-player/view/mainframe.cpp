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

#include <DUtil>
#include <DWidgetUtil>
#include <DAboutDialog>
#include <DDialog>
#include <DApplication>
#include <DTitlebar>
#include <DImageButton>
#include <DFileDialog>

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

#include <DSettingsDialog>

DWIDGET_USE_NAMESPACE

const QString s_PropertyViewname = "viewname";
const QString s_PropertyViewnameLyric = "lyric";
static const int FooterHeight = 60;
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

    void setPlayListVisible(bool visible);
    void toggleLyricView();
    void togglePlaylist();
    void slideToImportView();
    void slideToLyricView();
    void slideToMusicListView(bool keepPlaylist);
    void disableControl(int delay = 350);
    void updateSize(QSize newSize);
    void updateViewname(const QString &vm);
    void updateTitlebarViewname(const QString &vm);
    void overrideTitlebarStyle();
    const QString getLastImportPath() const;

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

    QAction             *newSonglistAction      = nullptr;
    QAction             *colorModeAction        = nullptr;
    QString             coverBackground         = ":/common/image/cover_max.svg";
    QImage              originCoverImage;
    QImage              currentCoverImage;
    QString             viewname                = "";

    QPoint              m_LastMousePos;

    MainFrame *q_ptr;
    Q_DECLARE_PUBLIC(MainFrame)
};

void MainFramePrivate::initMenu()
{
    Q_Q(MainFrame);

    newSonglistAction = new QAction(MainFrame::tr("New playlist"), q);
    q->connect(newSonglistAction, &QAction::triggered, q, [ = ](bool) {
        Q_EMIT q->addPlaylist(true);
    });

    auto addmusic = new QAction(MainFrame::tr("Add folder"), q);
    q->connect(addmusic, &QAction::triggered, q, [ = ](bool) {
        q->onSelectImportDirectory();
    });

    auto addmusicfiles = new QAction(MainFrame::tr("Add file"), q);
    q->connect(addmusicfiles, &QAction::triggered, q, [ = ](bool) {
        q->onSelectImportFiles();
    });

    auto settings = new QAction(MainFrame::tr("Settings"), q);
    q->connect(settings, &QAction::triggered, q, [ = ](bool) {
        auto configDialog = new DSettingsDialog(q);
        configDialog->setProperty("_d_QSSThemename", "dark");
        configDialog->setProperty("_d_QSSFilename", "DSettingsDialog");

        configDialog->setFixedSize(720, 550);
        configDialog->updateSettings(MusicSettings::settings());

        WidgetHelper::workaround_updateStyle(configDialog, "dlight");
        Dtk::Widget::moveToCenter(configDialog);
        configDialog->exec();
        MusicSettings::sync();
    });

    colorModeAction = new QAction(MainFrame::tr("Dark theme"), q);
    colorModeAction->setCheckable(true);
    colorModeAction->setChecked(MusicSettings::value("base.play.theme").toString() == "dark");

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
    titleMenu->addAction(addmusic);
    titleMenu->addAction(addmusicfiles);
    titleMenu->addSeparator();

    //titleMenu->addAction(colorModeAction);
    titleMenu->addAction(settings);
    titleMenu->addSeparator();

    titlebar->setMenu(titleMenu);
}

void MainFramePrivate::initUI(bool showLoading)
{
    showLoading = true;
    Q_Q(MainFrame);
    q->setMinimumSize(QSize(720, 480));
    q->setFocusPolicy(Qt::ClickFocus);

    titlebarwidget = new TitlebarWidget(q);
    titlebarwidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    titlebar = q->titlebar();
    titlebar->setIcon(QIcon(":/mpimage/deepin_music_player.svg"));    //titlebar->setCustomWidget(titlebarwidget, Qt::AlignLeft, false);
    titlebar->addWidget(titlebarwidget, Qt::AlignLeft);
//    titlebar->setBackgroundTransparent(true);
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
    footer->setFixedHeight(FooterHeight + footer->progressExtentHeight());
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
    footer->show();

    infoDialog = new InfoDialog(q);
    infoDialog->hide();
}

void MainFramePrivate::postInitUI()
{
    Q_Q(MainFrame);

    QString descriptionText = MainFrame::tr("Deepin Music is a beautiful and simple music player that plays local audios. It supports viewing lyrics during playback, playing lossless audio and playlist customization, etc.");
    QString acknowledgementLink = "https://www.deepin.org/acknowledgments/deepin-music#thanks";
    qApp->setProductName(QApplication::tr("Deepin Music"));
    qApp->setApplicationAcknowledgementPage(acknowledgementLink);
    qApp->setProductIcon(QIcon(":/common/image/app_icon.svg"));
    qApp->setApplicationDescription(descriptionText);

    searchResult = new SearchResult(q);
    titlebarwidget->setResultWidget(searchResult);
    loadWidget->hide();

    playListWidget = new PlayListWidget;
    playListWidget->setContentsMargins(0, titlebar->height(), 0, FooterHeight);

    lyricWidget = new MUsicLyricWidget;
    lyricWidget->setContentsMargins(0, titlebar->height(), 0, FooterHeight);

    musicListWidget = new MusicListWidget;
    musicListWidget->setContentsMargins(0, titlebar->height(), 0, FooterHeight);

    contentLayout->addWidget(importWidget);
    contentLayout->addWidget(lyricWidget);
    contentLayout->addWidget(musicListWidget);
    contentLayout->addWidget(playListWidget);

    titlebarwidget->setSearchEnable(false);
    importWidget->show();
    footer->show();
    footer->setFocus();
    updateSize(q->size());
}

void MainFramePrivate::slideToLyricView()
{
//    Q_Q(MainFrame);

    auto current = currentWidget ? currentWidget : playListWidget;
    WidgetHelper::slideBottom2TopWidget(
        current,  lyricWidget, AnimationDelay);

//    q->disableControl();
//    setPlaylistVisible(false);
    currentWidget = lyricWidget;
    titlebar->raise();
    footer->raise();

    updateViewname(s_PropertyViewnameLyric);
}

void MainFramePrivate:: slideToImportView()
{
//    Q_Q(MainFrame);

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
    footer->raise();

    titlebarwidget->setSearchEnable(false);
    newSonglistAction->setDisabled(true);
    updateViewname("");
}

void MainFramePrivate:: slideToMusicListView(bool keepPlaylist)
{
    Q_Q(MainFrame);

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
    disableControl(AnimationDelay);
    currentWidget = musicListWidget;
    titlebar->raise();
    footer->raise();

    titlebarwidget->setSearchEnable(true);
    newSonglistAction->setDisabled(false);
    footer->setFocus();
    updateViewname("");
}

void MainFramePrivate::toggleLyricView()
{
    playListWidget->hide();
    if (lyricWidget->isVisible()) {
        slideToMusicListView(false);
    } else {
        slideToLyricView();
    }
}

void MainFramePrivate::togglePlaylist()
{
    importWidget->hide();
    slideToMusicListView(true);
    setPlayListVisible(!playListWidget->isVisible());
}

void MainFramePrivate::setPlayListVisible(bool visible)
{
    Q_Q(MainFrame);
    if (playListWidget->isVisible() == visible) {
        if (visible) {
            playListWidget->setFocus();
            playListWidget->show();
            playListWidget->raise();
        }
        return;
    }

    auto ismoving = playListWidget->property("moving").toBool();
    if (ismoving) {
        return;
    }

    playListWidget->setEnabled(false);
    playListWidget->setProperty("moving", true);
    auto titleBarHeight = titlebar->height();

    int delay = AnimationDelay * 6 / 10;
    QRect start(q->width(), 0,
                playListWidget->width(), playListWidget->height());
    QRect end(q->width() - playListWidget->width(), 0,
              playListWidget->width(), playListWidget->height());
    if (!visible) {
        WidgetHelper::slideEdgeWidget(playListWidget, end, start, delay, true);
        footer->setFocus();
    } else {
        playListWidget->setFocus();
        WidgetHelper::slideEdgeWidget(playListWidget, start, end, delay);
        playListWidget->raise();
    }
    disableControl(delay);
    titlebar->raise();
    footer->raise();

    QTimer::singleShot(delay * 1, q, [ = ]() {
        playListWidget->setProperty("moving", false);
    });
}

void MainFramePrivate::disableControl(int delay)
{
    Q_Q(MainFrame);
    footer->enableControl(false);
    QTimer::singleShot(delay, q, [ = ]() {
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
    titlebar->move(0, 0);
    titlebar->resize(newSize.width(), titleBarHeight);
    titlebarwidget->setFixedSize(newSize.width() - titlebar->buttonAreaWidth() - FooterHeight, titleBarHeight);

    importWidget->setFixedSize(newSize);

    auto progressExtHeight = footer->progressExtentHeight();
    if (lyricWidget) {
        lyricWidget->resize(newSize);
        playListWidget->setFixedSize(newSize);
        musicListWidget->setFixedSize(newSize);
    }

    footer->raise();
    footer->resize(newSize.width(), FooterHeight + progressExtHeight);
    footer->setFixedHeight(FooterHeight + progressExtHeight);
    footer->move(0, newSize.height() - FooterHeight - progressExtHeight);
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

void MainFramePrivate::showInfoDialog(const MetaPtr meta)
{
    infoDialog->show();
    infoDialog->updateInfo(meta);
}

MainFrame::MainFrame(QWidget *parent) :
    DMainWindow(parent), dd_ptr(new MainFramePrivate(this))
{
    setObjectName("MainFrame");
}

MainFrame::~MainFrame()
{

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
    trayIcon->setIcon(QIcon(":/common/image/deepin-music.svg"));
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

    connect(this, &MainFrame::importSelectFiles, presenter, &Presenter::onImportFiles);
    connect(this, &MainFrame::addPlaylist, presenter, &Presenter::onPlaylistAdd);

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

    connect(d->titlebarwidget, &TitlebarWidget::locateMusicInAllMusiclist,
            presenter, &Presenter::onLocateMusicAtAll);
    connect(d->titlebarwidget, &TitlebarWidget::search,
            presenter, &Presenter::onSearchText);
    connect(d->titlebarwidget, &TitlebarWidget::searchExited,
            presenter, &Presenter::onExitSearch);

    connect(d->importWidget, &ImportWidget::scanMusicDirectory,
            presenter, &Presenter::onScanMusicDirectory);
    connect(d->importWidget, &ImportWidget::importFiles,
            this, &MainFrame::onSelectImportDirectory);
    connect(d->importWidget, &ImportWidget::importSelectFiles,
    this, [ = ](const QStringList & urllist) {
        d->importWidget->showWaitHint();
        Q_EMIT importSelectFiles(urllist);
    });

    connect(presenter, &Presenter::notifyAddToPlaylist,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList) {
        auto icon = QIcon(":/common/image/notify_success.svg");
        QFontMetrics fm(font());
        auto displayName = fm.elidedText(playlist->displayName(), Qt::ElideMiddle, 300);
        auto text = tr("Successfully added to \"%1\"").arg(displayName);
        this->sendMessage(icon, text);
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
    });

    connect(presenter, &Presenter::notifyMusciError,
    this, [ = ](PlaylistPtr playlist, const MetaPtr  meta, int /*error*/) {
        Dtk::Widget::DDialog warnDlg(this);
        warnDlg.setIcon(QIcon(":/common/image/dialog_warning.svg"));
        warnDlg.setTextFormat(Qt::RichText);
        warnDlg.setTitle(tr("Invalid or non-existent file"));
        warnDlg.addButtons(QStringList() << tr("OK"));
        warnDlg.setDefaultButton(0);

        if (0 == warnDlg.exec()) {
            if (playlist->canNext() && playlist->playing() == meta) {
                Q_EMIT presenter->playNext(playlist, meta);
            }
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
            QString message = QString(tr("Failed to import, no vaild music file found!"));
            Dtk::Widget::DDialog warnDlg(this);
            warnDlg.setIcon(QIcon(":/common/image/dialog_warning.svg"));
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
    });

    connect(presenter, &Presenter::meidaFilesImported,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList metalist) {
        d->setPlayListVisible(false);
        d->playListWidget->onMusiclistChanged(playlist);
        d->musicListWidget->onMusiclistUpdate();
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

    connect(d->footer,  &Footer::locateMusic,
    this, [ = ](PlaylistPtr playlist, const MetaPtr) {
        if (playlist && playlist->id() != SearchMusicListID) {
            d->titlebarwidget->exitSearch();
        }
        if (d->lyricWidget->isVisible()) {
            d->setPlayListVisible(true);
        }
    });

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

    connect(d->playListWidget, &PlayListWidget::importSelectFiles,
    this, [ = ](PlaylistPtr playlist, QStringList urllist) {
        presenter->requestImportPaths(playlist, urllist);
    });
    connect(d->playListWidget, &PlayListWidget::updateMetaCodec,
            d->footer, &Footer::onUpdateMetaCodec);

    connect(presenter, &Presenter::musicListResorted,
            d->playListWidget, &PlayListWidget::onMusiclistChanged);
    connect(presenter, &Presenter::requestMusicListMenu,
            d->playListWidget,  &PlayListWidget::onCustomContextMenuRequest);
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

    connect(d->footer,  &Footer::progressRealHeightChanged,
    this, [ = ](qreal realHeight) {
        int margin = FooterHeight
                     - d->footer->progressExtentHeight()
                     + static_cast<int>(realHeight);
        d->playListWidget->setContentsMargins(0, d->titlebar->height(), 0, margin);
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
//    connect(d->musicListWidget,  &MusicListWidget::hidePlaylist,
//    this, [ = ]() {
//        d->setPlaylistVisible(false);
//    });

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
    if (DFileDialog::Accepted == fileDlg.exec()) {
        d->importWidget->showWaitHint();
        MusicSettings::setOption("base.play.last_import_path",  fileDlg.directory().path());
        Q_EMIT importSelectFiles(fileDlg.selectedFiles());
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
        Q_EMIT importSelectFiles(fileDlg.selectedFiles());
    }
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
//                qDebug() << "match " << optkey << ke->count() << ke->isAutoRepeat();
                Q_EMIT  triggerShortcutAction(optkey);
                return true;
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

    p.drawImage(rect(), d->currentCoverImage);
    DMainWindow::paintEvent(e);
}
