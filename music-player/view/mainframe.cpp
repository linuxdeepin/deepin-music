/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * q program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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

#include <DUtil>
#include <dutility.h>
#include <daboutdialog.h>
#include <dsettingsdialog.h>
#include <ddialog.h>
#include <DApplication>

#include "../presenter/presenter.h"
#include "../core/metasearchservice.h"
#include "../core/settings.h"
#include "../core/player.h"
#include "../musicapp.h"

#include "widget/titlebarwidget.h"
#include "widget/infodialog.h"
#include "widget/tip.h"
#include "helper/widgethellper.h"
#include "helper/thememanager.h"

#include "titlebar.h"
#include "importwidget.h"
#include "musiclistwidget.h"
#include "playlistwidget.h"
#include "lyricwidget.h"
#include "footerwidget.h"

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

    void initUI();
    void initMenu();

    void setPlaylistVisible(bool visible);
    void toggleLyricView();
    void togglePlaylist();
    void slideToImportView();
    void slideToLyricView();
    void slideToMusicListView(bool keepPlaylist);
    void showTips(QPixmap icon, QString text);
    void disableControl(int delay = 350);
    void updateViewname(const QString &vm);

    //! ui: show info dialog
    void showInfoDialog(const MetaPtr meta);

    QWidget         *centralWidget  = nullptr;
    Titlebar        *titlebar       = nullptr;
    Tip             *tips           = nullptr;
    TitleBarWidget  *titlebarwidget = nullptr;
    ImportWidget    *importWidget   = nullptr;
    MusicListWidget *musicList      = nullptr;
    LyricWidget     *lyricWidget    = nullptr;
    PlaylistWidget  *playlistWidget = nullptr;
    Footer          *footer         = nullptr;

    QWidget         *currentWidget  = nullptr;
    InfoDialog      *infoDialog     = nullptr;

    QAction         *newSonglistAction      = nullptr;
    QAction         *colorModeAction        = nullptr;
    QString         coverBackground         = ":/common/image/cover_max.png";
    QImage          coverImage;
    QString         viewname                = "";

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

    auto addmusic = new QAction(MainFrame::tr("Add music"), q);
    q->connect(addmusic, &QAction::triggered, q, [ = ](bool) {
        q->onSelectImportDirectory();
    });

    auto addmusicfiles = new QAction(MainFrame::tr("Add music file"), q);
    q->connect(addmusicfiles, &QAction::triggered, q, [ = ](bool) {
        q->onSelectImportFiles();
    });

    auto settings = new QAction(MainFrame::tr("Settings"), q);
    q->connect(settings, &QAction::triggered, q, [ = ](bool) {
        auto configDialog = new Dtk::Widget::DSettingsDialog(q);
        configDialog->setProperty("_d_QSSThemename", "dark");
        configDialog->setProperty("_d_QSSFilename", "DSettingsDialog");
        ThemeManager::instance()->regisetrWidget(configDialog);

        configDialog->setFixedSize(720, 520);
        configDialog->updateSettings(AppSettings::instance()->settings());

        WidgetHelper::workaround_updateStyle(configDialog, "dlight");
        Dtk::Widget::DUtility::moveToCenter(configDialog);
        configDialog->exec();
        AppSettings::instance()->sync();
    });

    colorModeAction = new QAction(MainFrame::tr("Deep color mode"), q);
    colorModeAction->setCheckable(true);
    colorModeAction->setChecked(AppSettings::instance()->value("base.play.theme").toString() == "dark");

    q->connect(colorModeAction, &QAction::triggered, q, [ = ](bool) {
        if (ThemeManager::instance()->theme() == "light") {
            colorModeAction->setChecked(true);
            ThemeManager::instance()->setTheme("dark");
        } else {
            colorModeAction->setChecked(false);
            ThemeManager::instance()->setTheme("light");
        }
        AppSettings::instance()->setOption("base.play.theme", ThemeManager::instance()->theme());
    });

    QAction *m_close = new QAction(MainFrame::tr("Exit"), q);
    q->connect(m_close, &QAction::triggered, q, [ = ](bool) {
        q->close();
    });

    auto titleMenu = new QMenu;
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

void MainFramePrivate::initUI()
{
    Q_Q(MainFrame);

    q->setFocusPolicy(Qt::ClickFocus);
    q->setObjectName("MainFrame");

    titlebar =  new Titlebar;
    q->setMenuWidget(titlebar);

    titlebarwidget = new TitleBarWidget;
    titlebarwidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    titlebar->setCustomWidget(titlebarwidget , Qt::AlignLeft, false);

    importWidget = new ImportWidget;
    musicList = new MusicListWidget;
    musicList->setContentsMargins(0, titlebar->height(), 0, FooterHeight);

    lyricWidget = new LyricWidget;
    lyricWidget->setContentsMargins(0, titlebar->height(), 0, FooterHeight);

    playlistWidget = new PlaylistWidget;

    footer = new Footer;
    footer->setFixedHeight(FooterHeight);
    footer->enableControl(false);

    centralWidget = new QWidget;
    auto contentLayout = new QStackedLayout(centralWidget);
    q->setCentralWidget(centralWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);

    contentLayout->addWidget(titlebar);
    contentLayout->addWidget(importWidget);
    contentLayout->addWidget(musicList);
    contentLayout->addWidget(lyricWidget);
    contentLayout->addWidget(playlistWidget);
    contentLayout->addWidget(footer);

    infoDialog = new InfoDialog;
    infoDialog->setStyle(QStyleFactory::create("dlight"));
    infoDialog->hide();

    titlebarwidget->setSearchEnable(false);
    importWidget->show();
    footer->show();
    footer->setFocus();
}

void MainFramePrivate::slideToLyricView()
{
    Q_Q(MainFrame);

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
    Q_Q(MainFrame);

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

void MainFramePrivate::showTips(QPixmap icon, QString text)
{
    Q_Q(MainFrame);
    if (tips) {
        tips->hide();
        tips->deleteLater();
    }

    tips = new Tip(icon, text , q);
    auto center = q->mapToGlobal(QPoint(q->rect().center()));
    center.setY(center.y() + q->height() / 2 - footer->height() - 40 - 36);
    center = tips->mapFromGlobal(center);
    center = tips->mapToParent(center);
    tips->pop(center);
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

    qDebug() << "start move";

    playlistWidget->setEnabled(false);
    playlistWidget->setProperty("moving", true);
    auto titleBarHeight = titlebar->height();

    double factor = 0.6;
    QRect start(q->width(), titleBarHeight,
                playlistWidget->width(), playlistWidget->height());
    QRect end(q->width() - playlistWidget->width(), titleBarHeight,
              playlistWidget->width(), playlistWidget->height());
    if (!visible) {
        WidgetHelper::slideEdgeWidget(playlistWidget, end, start, AnimationDelay * factor, true);
        footer->setFocus();
    } else {
        playlistWidget->setFocus();
        WidgetHelper::slideEdgeWidget(playlistWidget, start, end, AnimationDelay * factor);
        playlistWidget->raise();
    }
    disableControl(AnimationDelay * factor);
    titlebar->raise();
    footer->raise();

    QTimer::singleShot(AnimationDelay * factor * 1, q, [ = ]() {
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

void MainFramePrivate::updateViewname(const QString &vm)
{
    Q_Q(MainFrame);
    DUtil::TimerSingleShot(AnimationDelay / 2, [this, q, vm]() {
        titlebar->setViewname(vm);
        titlebarwidget->setViewname(vm);
    });
}

void MainFramePrivate::showInfoDialog(const MetaPtr meta)
{
    infoDialog->show();
    infoDialog->updateInfo(meta);
}

MainFrame::MainFrame(QWidget *parent) :
    DMainWindow(parent), d_ptr(new MainFramePrivate(this))
{
    Q_D(MainFrame);
    ThemeManager::instance()->regisetrWidget(this, QStringList() << s_PropertyViewname);

    d->initUI();
    d->initMenu();

    // TODO: workaround
    if (ThemeManager::instance()->theme() == "dark") {
        this->setBorderColor(QColor("#101010"));
    }
    connect(ThemeManager::instance(), &ThemeManager::themeChanged,
    this, [ = ](QString theme) {
        if (theme == "dark") {
            this->setBorderColor(QColor("#101010"));
        } else {
            this->setBorderColor(Qt::white);
        }
    });
}

MainFrame::~MainFrame()
{

}

void MainFrame::binding(Presenter *presenter)
{
    Q_D(MainFrame);

    connect(this, &MainFrame::importSelectFiles, presenter, &Presenter::onImportFiles);
    connect(this, &MainFrame::addPlaylist, presenter, &Presenter::onPlaylistAdd);

//    connect(d->titlebar, &Titlebar::mouseMoving, this, &MainFrame::moveWindow);
//    connect(d->footer, &Footer::mouseMoving, this, &MainFrame::moveWindow);

    connect(d->titlebarwidget, &TitleBarWidget::locateMusicInAllMusiclist,
            presenter, &Presenter::onLocateMusicAtAll);
    connect(d->titlebarwidget, &TitleBarWidget::search,
            presenter, &Presenter::onSearchText);
    connect(d->titlebarwidget, &TitleBarWidget::searchExited,
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
        auto icon = QPixmap(":/common/image/notify_success.png");
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

        d->coverImage = WidgetHelper::blurImage(image, BlurRadius).toImage();
        image = WidgetHelper::cropRect(d->coverImage, size());
        QPalette palette;
        palette.setBrush(QPalette::Background, image);
        setPalette(palette);
        update();
    });

    connect(presenter, &Presenter::musicStoped,
    this, [ = ](PlaylistPtr, const MetaPtr) {
        setCoverBackground(coverBackground());
    });

    connect(presenter, &Presenter::notifyMusciError,
    this, [ = ](PlaylistPtr playlist, const MetaPtr  meta, int /*error*/) {
        Dtk::Widget::DDialog warnDlg(this);
        warnDlg.setIcon(QIcon(":/common/image/dialog_warning.png"));
        warnDlg.setTextFormat(Qt::RichText);
        warnDlg.setTitle(tr("File invalid or does not exist, failed to load!"));
        warnDlg.addButtons(QStringList() << tr("I got it"));
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
            warnDlg.setIcon(QIcon(":/common/image/dialog_warning.png"));
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
    image = WidgetHelper::blurImage(image, BlurRadius).toImage();
    image = WidgetHelper::cropRect(image, size());
    d->coverImage = image;

    QPalette palette;
    palette.setBrush(QPalette::Background, image);
    setPalette(palette);

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
    fileDlg.setFileMode(QFileDialog::Directory);
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

    QString selfilter = tr("Music (%1)");
    selfilter = selfilter.arg(Player::instance()->supportedSuffixList().join(" "));
    fileDlg.setViewMode(QFileDialog::Detail);
    fileDlg.setFileMode(QFileDialog::ExistingFiles);

    fileDlg.setNameFilter(selfilter);
    fileDlg.selectNameFilter(selfilter);
    if (QFileDialog::Accepted == fileDlg.exec()) {
        d->importWidget->showWaitHint();
        AppSettings::instance()->setOption("base.play.last_import_path",  fileDlg.directory().path());
        emit importSelectFiles(fileDlg.selectedFiles());
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
            auto shortcut = AppSettings::instance()->value(optkey).toStringList();
            auto modifiersstr = shortcut.value(0);
            auto scmodifiers = static_cast<Qt::KeyboardModifier>(modifiersstr.toInt());
            auto keystr = shortcut.value(1);
            auto sckey = static_cast<Qt::Key>(keystr.toInt());

            if (scmodifiers == keyModifiers && key == sckey && !ke->isAutoRepeat()) {
//                qDebug() << "match " << optkey << ke->count() << ke->isAutoRepeat();
                MusicApp::instance()->triggerShortcutAction(optkey);
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

void MainFrame::resizeEvent(QResizeEvent *e)
{
    Q_D(const MainFrame);

    DMainWindow::resizeEvent(e);
    QSize newSize = DMainWindow::size();

    auto titleBarHeight =  d->titlebar->height();
    d->titlebar->raise();
    d->titlebar->move(0, 0);
    d->titlebar->resize(newSize.width(), titleBarHeight);
    d->titlebarwidget->setFixedSize(newSize.width() - d->titlebar->buttonAreaWidth() - 60, titleBarHeight);

    d->importWidget->setFixedSize(newSize);
    d->lyricWidget->resize(newSize);
    d->musicList->setFixedSize(newSize);

    d->playlistWidget->setFixedSize(220, newSize.height() - FooterHeight - titleBarHeight);
    if (d->playlistWidget->isVisible()) {
        d->playlistWidget->hide();
    }

    d->footer->raise();
    d->footer->resize(newSize.width(), FooterHeight);
    d->footer->move(0, newSize.height() - FooterHeight);

    if (d->tips) {
        d->tips->hide();
    }

    if (!d->coverImage.isNull()) {
        auto image = WidgetHelper::cropRect(d->coverImage, size());
        QPalette palette;
        palette.setBrush(QPalette::Background, image);
        setPalette(palette);
    }
}

void MainFrame::closeEvent(QCloseEvent *event)
{
    MusicApp::instance()->deleteLater();
    AppSettings::instance()->setOption("base.play.state", int(windowState()));
    AppSettings::instance()->setOption("base.play.geometry", saveGeometry());
    //    qDebug() << "store state:" << windowState() << "gometry:" << geometry();
    DMainWindow::closeEvent(event);
}
