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
#include <QStyleFactory>
#include <QFileDialog>
#include <QStandardPaths>
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QSystemTrayIcon>

#include <DUtil>
#include <DWidgetUtil>
#include <daboutdialog.h>
#include <ddialog.h>
#include <DApplication>
#include <DThemeManager>
#include <DToast>

#include "../presenter/presenter.h"
#include "../core/metasearchservice.h"
#include "../core/settings.h"
#include "../core/player.h"
#include "../musicapp.h"

#include "widget/titlebarwidget.h"
#include "widget/infodialog.h"
#include "widget/searchresult.h"
#include "widget/closeconfirmdialog.h"
#include "helper/widgethellper.h"

#include "titlebar.h"
#include "importwidget.h"
#include "musiclistwidget.h"
#include "playlistwidget.h"
#include "lyricwidget.h"
#include "footerwidget.h"
#include "loadwidget.h"

#include <DSettingsDialog>

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

DWIDGET_USE_NAMESPACE

const QString s_PropertyViewname = "viewname";
const QString s_PropertyViewnameLyric = "lyric";
static const int FooterHeight = 60;
static const int AnimationDelay = 400; //ms
static const int BlurRadius = 25; //ms

using namespace Dtk::Widget;

class MainFramePrivate
{
public:
    MainFramePrivate(MainFrame *parent) : q_ptr(parent) {}

    void initUI(bool showLoading);
    void postInitUI();
    void initMenu();

    void setPlaylistVisible(bool visible);
    void toggleLyricView();
    void togglePlaylist();
    void slideToImportView();
    void slideToLyricView();
    void slideToMusicListView(bool keepPlaylist);
    void showTips(QIcon icon, QString text);
    void disableControl(int delay = 350);
    void updateSize(QSize newSize);
    void updateViewname(const QString &vm);
    void updateTitlebarViewname(const QString &vm);
    void overrideTitlebarStyle();

    //! ui: show info dialog
    void showInfoDialog(const MetaPtr meta);

    QWidget         *centralWidget  = nullptr;
    QStackedLayout  *contentLayout  = nullptr;
    DTitlebar       *titlebar       = nullptr;
    DToast          *tips           = nullptr;
    SearchResult    *searchResult   = nullptr;
    TitlebarWidget  *titlebarwidget = nullptr;
    ImportWidget    *importWidget   = nullptr;
    LoadWidget      *loadWidget     = nullptr;
    MusicListWidget *musicList      = nullptr;
    LyricWidget     *lyricWidget    = nullptr;
    PlaylistWidget  *playlistWidget = nullptr;
    Footer          *footer         = nullptr;

    QWidget         *currentWidget  = nullptr;
    InfoDialog      *infoDialog     = nullptr;

    QAction         *newSonglistAction      = nullptr;
    QAction         *colorModeAction        = nullptr;
    QString         coverBackground         = ":/common/image/cover_max.svg";
    QImage          originCoverImage;
    QImage          currentCoverImage;
    QString         viewname                = "";

    QPoint              m_LastMousePos;

    MainFrame *q_ptr;
    Q_DECLARE_PUBLIC(MainFrame)
};

void MainFramePrivate::initMenu()
{
    Q_Q(MainFrame);

    newSonglistAction = new QAction(MainFrame::tr("New playlist"), q);
    q->connect(newSonglistAction, &QAction::triggered, q, [ = ](bool) {
        setPlaylistVisible(true);
        emit q->addPlaylist(true);
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
        DThemeManager::instance()->registerWidget(configDialog);

        configDialog->setFixedSize(720, 520);
        configDialog->updateSettings(AppSettings::instance()->settings());

        WidgetHelper::workaround_updateStyle(configDialog, "dlight");
        Dtk::Widget::moveToCenter(configDialog);
        configDialog->exec();
        AppSettings::instance()->sync();
    });

    colorModeAction = new QAction(MainFrame::tr("Dark theme"), q);
    colorModeAction->setCheckable(true);
    colorModeAction->setChecked(AppSettings::instance()->value("base.play.theme").toString() == "dark");

    q->connect(colorModeAction, &QAction::triggered, q, [ = ](bool) {
        if (DThemeManager::instance()->theme() == "light") {
            colorModeAction->setChecked(true);
            DThemeManager::instance()->setTheme("dark");
        } else {
            colorModeAction->setChecked(false);
            DThemeManager::instance()->setTheme("light");
        }
        AppSettings::instance()->setOption("base.play.theme", DThemeManager::instance()->theme());
    });

    QAction *m_close = new QAction(MainFrame::tr("Exit"), q);
    q->connect(m_close, &QAction::triggered, q, [ = ](bool) {
        q->close();
    });

    auto titleMenu = new QMenu(q);
    titleMenu->setStyle(QStyleFactory::create("dlight"));

    titleMenu->addAction(newSonglistAction);
    titleMenu->addAction(addmusic);
    titleMenu->addAction(addmusicfiles);
    titleMenu->addSeparator();

    titleMenu->addAction(colorModeAction);
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
    titlebar->setCustomWidget(titlebarwidget, Qt::AlignLeft, false);
//    titlebar->setBackgroundTransparent(true);
    overrideTitlebarStyle();

    centralWidget = new QWidget(q);
    contentLayout = new QStackedLayout(centralWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
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

    QString descriptionText = MainFrame::tr("Deepin Music Player is a local  music player with beautiful design and simple functions. It supports viewing lyrics when playing, playing lossless music and customize playlist, etc.");
    QString acknowledgementLink = "https://www.deepin.org/acknowledgments/deepin-music#thanks";
    qApp->setProductName(QApplication::tr("Deepin Music"));
    qApp->setApplicationAcknowledgementPage(acknowledgementLink);
    qApp->setProductIcon(QIcon(":/common/image/app_icon.svg"));
    qApp->setApplicationDescription(descriptionText);

    searchResult = new SearchResult(q);
    titlebarwidget->setResultWidget(searchResult);
    loadWidget->hide();

    musicList = new MusicListWidget;
    musicList->setContentsMargins(0, titlebar->height(), 0, FooterHeight);

    lyricWidget = new LyricWidget;
    lyricWidget->setContentsMargins(0, titlebar->height(), 0, FooterHeight);

    playlistWidget = new PlaylistWidget;

    contentLayout->addWidget(importWidget);
    contentLayout->addWidget(musicList);
    contentLayout->addWidget(lyricWidget);
    contentLayout->addWidget(playlistWidget);

    titlebarwidget->setSearchEnable(false);
    importWidget->show();
    footer->show();
    footer->setFocus();
    updateSize(q->size());
}

void MainFramePrivate::slideToLyricView()
{
//    Q_Q(MainFrame);

    auto current = currentWidget ? currentWidget : musicList;
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

//    setPlaylistVisible(false);
    auto current = currentWidget ? currentWidget : musicList;
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
    if (musicList->isVisible()) {
        musicList->raise();
        titlebar->raise();
        footer->raise();
        setPlaylistVisible(keepPlaylist);
        return;
    }
    musicList->setFixedSize(current->size());
    WidgetHelper::slideTop2BottomWidget(
        current, musicList, AnimationDelay);
    q->update();
    disableControl(AnimationDelay);
    currentWidget = musicList;
    setPlaylistVisible(keepPlaylist);
    titlebar->raise();
    footer->raise();

    titlebarwidget->setSearchEnable(true);
    newSonglistAction->setDisabled(false);
    footer->setFocus();
    updateViewname("");
}

void MainFramePrivate::showTips(QIcon icon, QString text)
{
    Q_Q(MainFrame);
    if (tips) {
        tips->hide();
        tips->deleteLater();
    }

    tips = new DToast(q);
    tips->setIcon(icon);
    tips->setText(text);
    tips->pop();
    auto center = q->mapToGlobal(QPoint(q->rect().center()));
    center.setX(center.x() - tips->width() / 2);
    center.setY(center.y() + q->height() / 2 - footer->height() - 40);
    center = tips->mapFromGlobal(center);
    center = tips->mapToParent(center);
    tips->move(center);
}

void MainFramePrivate::toggleLyricView()
{
    if (lyricWidget->isVisible()) {
        slideToMusicListView(false);
    } else {
        slideToLyricView();
    }
}

void MainFramePrivate::togglePlaylist()
{
    setPlaylistVisible(!playlistWidget->isVisible());
}

void MainFramePrivate::setPlaylistVisible(bool visible)
{
    Q_Q(MainFrame);
    if (playlistWidget->isVisible() == visible) {
        if (visible) {
            playlistWidget->setFocus();
            playlistWidget->show();
            playlistWidget->raise();
        }
        return;
    }

    auto ismoving = playlistWidget->property("moving").toBool();
    if (ismoving) {
        return;
    }

    playlistWidget->setEnabled(false);
    playlistWidget->setProperty("moving", true);
    auto titleBarHeight = titlebar->height();

    int delay = AnimationDelay * 6 / 10;
    QRect start(q->width(), titleBarHeight,
                playlistWidget->width(), playlistWidget->height());
    QRect end(q->width() - playlistWidget->width(), titleBarHeight,
              playlistWidget->width(), playlistWidget->height());
    if (!visible) {
        WidgetHelper::slideEdgeWidget(playlistWidget, end, start, delay, true);
        footer->setFocus();
    } else {
        playlistWidget->setFocus();
        WidgetHelper::slideEdgeWidget(playlistWidget, start, end, delay);
        playlistWidget->raise();
    }
    disableControl(delay);
    titlebar->raise();
    footer->raise();

    QTimer::singleShot(delay * 1, q, [ = ]() {
        playlistWidget->setProperty("moving", false);
    });
}

void MainFramePrivate::disableControl(int delay)
{
    Q_Q(MainFrame);
    footer->enableControl(false);
    QTimer::singleShot(delay, q, [ = ]() {
        footer->enableControl(true);
        playlistWidget->setEnabled(true);
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
        musicList->setFixedSize(newSize);

        playlistWidget->setFixedSize(220, newSize.height() - FooterHeight - titleBarHeight);
        if (playlistWidget->isVisible()) {
            playlistWidget->hide();
        }
    }

    footer->raise();
    footer->resize(newSize.width(), FooterHeight + progressExtHeight);
    footer->move(0, newSize.height() - FooterHeight - progressExtHeight);

    if (tips) {
        tips->hide();
    }
}

void MainFramePrivate::updateViewname(const QString &vm)
{
    Q_Q(MainFrame);
    DUtil::TimerSingleShot(AnimationDelay / 2, [this, q, vm]() {
        updateTitlebarViewname(vm);
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
    DThemeManager::instance()->registerWidget(titlebar, "Titlebar", QStringList({"viewname"}));

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
        titlebarBt->setProperty("_d_QSSFilename", "Titlebar");
        DThemeManager::instance()->registerWidget(titlebarBt, QStringList({"viewname"}));
    }
}

void MainFramePrivate::showInfoDialog(const MetaPtr meta)
{
    infoDialog->show();
    infoDialog->updateInfo(meta);
}

MainFrame::MainFrame(QWidget *parent) :
    DMainWindow(parent), d_ptr(new MainFramePrivate(this))
{
    setObjectName("MainFrame");
    DThemeManager::instance()->registerWidget(this, QStringList() << s_PropertyViewname);
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
    focusMusicList();

    auto playAction = new QAction(tr("Play/Pause"), this);
    auto prevAction = new QAction(tr("Previous"), this);
    auto nextAction = new QAction(tr("Next"), this);
    auto quitAction = new QAction(tr("Exit"), this);

    auto trayIconMenu = new QMenu(this);
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
        Q_EMIT triggerShortcutAction("shortcuts.all.play_pause");
    });
    connect(prevAction, &QAction::triggered,
    this, [ = ]() {
        Q_EMIT triggerShortcutAction("shortcuts.all.previous");
    });
    connect(nextAction, &QAction::triggered,
    this, [ = ]() {
        Q_EMIT triggerShortcutAction("shortcuts.all.next");
    });
    connect(quitAction, &QAction::triggered,
    this, [ = ]() {
        onQuit();
        qApp->quit();
    });
    connect(trayIcon, &QSystemTrayIcon::activated,
    this, [ = ](QSystemTrayIcon::ActivationReason reason) {
        if (QSystemTrayIcon::Trigger == reason) {
            if (isMinimized()) {
                // FIXME: why??? showNormal not work if do not hide and show
                setVisible(false);
                setVisible(true);

                showNormal();
            } else {
                showMinimized();
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
    this, [ = ](Qt::MouseButton /*botton*/, QPoint pos) {
        move(pos - d->m_LastMousePos);
    });

    connect(d->titlebar, &Titlebar::mousePosPressed,
    this, [ = ](Qt::MouseButtons /*botton*/, QPoint pos) {
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
        emit importSelectFiles(urllist);
    });

    connect(presenter, &Presenter::notifyAddToPlaylist,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList) {
        auto icon = QIcon(":/common/image/notify_success.svg");
        QFontMetrics fm(font());
        auto displayName = fm.elidedText(playlist->displayName(), Qt::ElideMiddle, 300);
        auto text = tr("Successfully added to \"%1\"").arg(displayName);
        d->showTips(icon, text);
    });

    connect(presenter, &Presenter::showPlaylist,
    this, [ = ](bool show) {
        d->setPlaylistVisible(show);
    });

    connect(presenter, &Presenter::showMusicList,
    this, [ = ](PlaylistPtr playlist) {
        auto current = d->currentWidget ? d->currentWidget : d->importWidget;
        d->musicList->resize(current->size());
        d->musicList->show();
        d->importWidget->hide();
        d->musicList->onMusiclistChanged(playlist);
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
                emit presenter->playNext(playlist, meta);
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
        d->slideToMusicListView(false);
        d->musicList->onMusiclistChanged(playlist);
        if (!metalist.isEmpty()) {
            d->musicList->setCustomSortType();
        }
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
            d->slideToMusicListView(false);
        }
    });

    connect(d->footer,  &Footer::locateMusic,
    this, [ = ](PlaylistPtr playlist, const MetaPtr) {
        if (playlist && playlist->id() != SearchMusicListID) {
            d->titlebarwidget->exitSearch();
        }
        if (d->lyricWidget->isVisible()) {
            d->slideToMusicListView(false);
        }
    });

    // MusicList
    connect(d->musicList, &MusicListWidget::showInfoDialog,
    this, [ = ](const MetaPtr meta) {
        d->showInfoDialog(meta);
    });

    connect(d->musicList, &MusicListWidget::playall,
            presenter, &Presenter::onPlayall);
    connect(d->musicList, &MusicListWidget::resort,
            presenter, &Presenter::onResort);
    connect(d->musicList, &MusicListWidget::playMedia,
            presenter, &Presenter::onSyncMusicPlay);
    connect(d->musicList, &MusicListWidget::updateMetaCodec,
            presenter, &Presenter::onUpdateMetaCodec);
    connect(d->musicList, &MusicListWidget::requestCustomContextMenu,
            presenter, &Presenter::onRequestMusiclistMenu);
    connect(d->musicList, &MusicListWidget::addToPlaylist,
            presenter, &Presenter::onAddToPlaylist);
    connect(d->musicList, &MusicListWidget::musiclistRemove,
            presenter, &Presenter::onMusiclistRemove);
    connect(d->musicList, &MusicListWidget::musiclistDelete,
            presenter, &Presenter::onMusiclistDelete);

    connect(d->musicList, &MusicListWidget::importSelectFiles,
    this, [ = ](PlaylistPtr playlist, QStringList urllist) {
        presenter->requestImportPaths(playlist, urllist);
    });
    connect(d->musicList, &MusicListWidget::updateMetaCodec,
            d->footer, &Footer::onUpdateMetaCodec);

    connect(presenter, &Presenter::musicListResorted,
            d->musicList, &MusicListWidget::onMusiclistChanged);
    connect(presenter, &Presenter::requestMusicListMenu,
            d->musicList,  &MusicListWidget::onCustomContextMenuRequest);
    connect(presenter, &Presenter::currentMusicListChanged,
            d->musicList,  &MusicListWidget::onMusiclistChanged);
    connect(presenter, &Presenter::musicPlayed,
            d->musicList,  &MusicListWidget::onMusicPlayed);
    connect(presenter, &Presenter::musicError,
            d->musicList,  &MusicListWidget::onMusicError);
    connect(presenter, &Presenter::musicListAdded,
            d->musicList,  &MusicListWidget::onMusicListAdded);
    connect(presenter, &Presenter::musicListRemoved,
            d->musicList,  &MusicListWidget::onMusicListRemoved);
    connect(presenter, &Presenter::locateMusic,
            d->musicList,  &MusicListWidget::onLocate);

    connect(presenter, &Presenter::progrossChanged,
            d->lyricWidget, &LyricWidget::onProgressChanged);
    connect(presenter, &Presenter::musicPlayed,
            d->lyricWidget, &LyricWidget::onMusicPlayed);
    connect(presenter, &Presenter::coverSearchFinished,
            d->lyricWidget, &LyricWidget::onCoverChanged);
    connect(presenter, &Presenter::lyricSearchFinished,
            d->lyricWidget, &LyricWidget::onLyricChanged);
    connect(presenter, &Presenter::contextSearchFinished,
            d->lyricWidget, &LyricWidget::onContextSearchFinished);
    connect(presenter, &Presenter::musicStoped,
            d->lyricWidget,  &LyricWidget::onMusicStop);

    connect(d->lyricWidget,  &LyricWidget::requestContextSearch,
            presenter, &Presenter::requestContextSearch);
    connect(d->lyricWidget, &LyricWidget::changeMetaCache,
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
            d->musicList, &MusicListWidget::onLocate);
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

    // playlist
    connect(presenter, &Presenter::playlistAdded,
            d->playlistWidget,  &PlaylistWidget::onPlaylistAdded);
    connect(presenter, &Presenter::musicPlayed,
            d->playlistWidget,  &PlaylistWidget::onMusicPlayed);
    connect(presenter, &Presenter::currentMusicListChanged,
            d->playlistWidget,  &PlaylistWidget::onCurrentChanged);

    connect(d->playlistWidget,  &PlaylistWidget::addPlaylist,
            presenter, &Presenter::onPlaylistAdd);
    connect(d->playlistWidget,  &PlaylistWidget::selectPlaylist,
            presenter, &Presenter::onCurrentPlaylistChanged);
    connect(d->playlistWidget,  &PlaylistWidget::playall,
            presenter, &Presenter::onPlayall);
    connect(d->playlistWidget,  &PlaylistWidget::customResort,
            presenter, &Presenter::onCustomResort);
    connect(d->playlistWidget,  &PlaylistWidget::hidePlaylist,
    this, [ = ]() {
        d->setPlaylistVisible(false);
    });

}

void MainFrame::focusMusicList()
{
    Q_D(const MainFrame);
    d->musicList->setFocus();
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
    QFileDialog fileDlg(this);

    auto lastImportPath = AppSettings::instance()->value("base.play.last_import_path").toString();

    auto lastImportDir = QDir(lastImportPath);
    if (!lastImportDir.exists()) {
        lastImportPath =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    }
    fileDlg.setDirectory(lastImportPath);

    fileDlg.setViewMode(QFileDialog::Detail);
    fileDlg.setFileMode(QFileDialog::DirectoryOnly);
    if (QFileDialog::Accepted == fileDlg.exec()) {
        d->importWidget->showWaitHint();
        AppSettings::instance()->setOption("base.play.last_import_path",  fileDlg.directory().path());
        emit importSelectFiles(fileDlg.selectedFiles());
    }
}

void MainFrame::onSelectImportFiles()
{
    Q_D(const MainFrame);
    QFileDialog fileDlg(this);

    auto lastImportPath = AppSettings::instance()->value("base.play.last_import_path").toString();

    auto lastImportDir = QDir(lastImportPath);
    if (!lastImportDir.exists()) {
        lastImportPath =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    }
    fileDlg.setDirectory(lastImportPath);

    QString selfilter = tr("All music") + (" (%1)");
    selfilter = selfilter.arg(Player::instance()->supportedSuffixList().join(" "));
    fileDlg.setViewMode(QFileDialog::Detail);
    fileDlg.setFileMode(QFileDialog::ExistingFiles);
    fileDlg.setOption(QFileDialog::HideNameFilterDetails);

    fileDlg.setNameFilter(selfilter);
    fileDlg.selectNameFilter(selfilter);
    if (QFileDialog::Accepted == fileDlg.exec()) {
        d->importWidget->showWaitHint();
        AppSettings::instance()->setOption("base.play.last_import_path",  fileDlg.directory().path());
        emit importSelectFiles(fileDlg.selectedFiles());
    }
}

void MainFrame::onQuit()
{
    qDebug() << "sync config start";
    AppSettings::instance()->sync();
#ifdef Q_OS_LINUX
    sync();
#endif
    qDebug() << "sync config finish, app exit";
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
            auto shortcut = AppSettings::instance()->value(optkey).toStringList();
            auto modifiersstr = shortcut.value(0);
            auto scmodifiers = static_cast<Qt::KeyboardModifier>(modifiersstr.toInt());
            auto keystr = shortcut.value(1);
            auto sckey = static_cast<Qt::Key>(keystr.toInt());

            if (scmodifiers == keyModifiers && key == sckey && !ke->isAutoRepeat()) {
//                qDebug() << "match " << optkey << ke->count() << ke->isAutoRepeat();
                Q_EMIT triggerShortcutAction(optkey);
                return true;
            }
        }
    }

    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        if (obj->objectName() == this->objectName() || this->objectName() + "Window" == obj->objectName()) {
            QPoint mousePos = me->pos();
            auto geometry = d->playlistWidget->geometry().marginsAdded(QMargins(0, 0, 40, 40));
            if (!geometry.contains(mousePos)) {
                DUtil::TimerSingleShot(50, [this]() {
                    this->d_func()->setPlaylistVisible(false);
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
    auto askCloseAction = AppSettings::instance()->value("base.close.ask_close_action").toBool();
    if (askCloseAction) {
        CloseConfirmDialog ccd(this);
        if (0 == ccd.exec()) {
            event->ignore();
            return;
        } else {
            AppSettings::instance()->setOption("base.close.ask_close_action", !ccd.isRemember());
            AppSettings::instance()->setOption("base.close.close_action", ccd.closeAction());
        }
    }

    AppSettings::instance()->setOption("base.play.state", int(windowState()));
    AppSettings::instance()->setOption("base.play.geometry", saveGeometry());
    DMainWindow::closeEvent(event);
}

void MainFrame::paintEvent(QPaintEvent *e)
{
    Q_D(MainFrame);
    QPainter p(this);

    p.drawImage(rect(), d->currentCoverImage);
    DMainWindow::paintEvent(e);
}
