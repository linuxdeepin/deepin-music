/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "mainwindow.h"

#include <QDebug>
#include <QPainter>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QTimer>
#include <QProcess>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QMenu>
#include <QGraphicsDropShadowEffect>
#include <QStyleFactory>

#include <DUtil>
#include <dutility.h>
#include <DTitlebar>
#include <dthememanager.h>
#include <DAboutDialog>

#include <thememanager.h>
#include "titlebar.h"
#include "footer.h"
#include "importwidget.h"
#include "lyricview.h"
#include "playlistwidget.h"
#include "musiclistwidget.h"
#include "widget/tip.h"
#include "widget/titlebarwidget.h"
#include "widget/dsettingdialog.h"
#include "core/dsettings.h"

#include "../core/music.h"
#include "../core/playlist.h"
#include "../musicapp.h"
#include "../presenter/presenter.h"

#include "helper/widgethellper.h"

DWIDGET_USE_NAMESPACE

const QString s_PropertyViewname = "viewname";
const QString s_PropertyViewnameLyric = "lyric";

static const int s_AnimationDelay   = 350;
static QColor s_lyricTitleTop       = QColor(0, 0, 0, 94);
static QColor s_lyriclTitleBottom   = QColor(0, 0, 0, 102);
static const int titleBarHeight = 40;
static const int footerHeight = 60;

class MainWindowPrivate
{
public:
    MainWindowPrivate() {}

    Titlebar        *titlebar       = nullptr;
    TitleBarWidget  *titlebarwidget = nullptr;
    Footer          *footer         = nullptr;
    PlaylistWidget  *playlist       = nullptr;
    ImportWidget    *import         = nullptr;
    MusicListWidget *musicList      = nullptr;
    LyricView       *lyricView      = nullptr;


    Tip             *tips           = nullptr;
    QWidget         *currentWidget  = nullptr;

    QAction         *newSonglistAction      = nullptr;
    QAction         *colorModeAction        = nullptr;
    QString         coverBackground = ":/common/image/cover_max.png";

};

MainWindow::MainWindow(QWidget *parent)
    : ThinWindow(parent), d(new MainWindowPrivate)
{
    setFocusPolicy(Qt::ClickFocus);
    setObjectName("PlayerFrame");

    auto contentLayout = new QStackedLayout();
    setContentLayout(contentLayout);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);

    d->titlebar = new Titlebar();
    d->titlebar->setObjectName("MainWindowTitlebar");
    d->titlebarwidget = new TitleBarWidget();
    d->titlebarwidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    d->titlebar->setFixedHeight(titleBarHeight);
    d->titlebar->setCustomWidget(d->titlebarwidget , Qt::AlignCenter, false);

//    connect(d->title, &DTitlebar::optionClicked, this, &MainWindow::optionClicked);

    d->import = new ImportWidget;
    d->musicList = new MusicListWidget;
    d->lyricView = new LyricView;
    d->playlist = new PlaylistWidget;
    d->playlist->setFixedWidth(220);

    d->footer = new Footer;
    d->footer->setFixedHeight(footerHeight);

    d->musicList->setContentsMargins(0, titleBarHeight, 0, footerHeight);
    d->lyricView->setContentsMargins(0, titleBarHeight, 0, footerHeight);

    contentLayout->addWidget(d->titlebar);

//    contentLayout->addWidget(d->titlebar);
    contentLayout->addWidget(d->import);
    contentLayout->addWidget(d->musicList);
    contentLayout->addWidget(d->lyricView);
    contentLayout->addWidget(d->playlist);
    contentLayout->addWidget(d->footer);

    auto *bodyShadow = new QGraphicsDropShadowEffect;
    bodyShadow->setBlurRadius(20.0);
    bodyShadow->setColor(QColor(0, 0, 0, 0.10 * 255));
    bodyShadow->setOffset(0, 5.0);
    this->setGraphicsEffect(bodyShadow);

    ThemeManager::instance()->regisetrWidget(this);

    d->footer->setFocus();

    connect(d->titlebar, &DTitlebar::mouseMoving, this, &MainWindow::moveWindow);
    connect(d->footer, &Footer::mouseMoving, this, &MainWindow::moveWindow);
    setMinimumSize(840, 640);
    resize(QSize(840, 640));
}

MainWindow::~MainWindow()
{

}

void MainWindow::initUI()
{
    d->lyricView->initUI();
}

void MainWindow::initMusiclist(PlaylistPtr allmusic, PlaylistPtr last)
{

    initMenu();

    if (allmusic.isNull() || 0 == allmusic->length()) {
        d->newSonglistAction->setDisabled(true);
        d->import->show();
        d->titlebarwidget->setSearchEnable(false);
        d->footer->enableControl(false);
        d->musicList->hide();
        qWarning() << "no music in all music list" << allmusic;
        d->musicList->initData(allmusic);
        return;
    }

    if (last.isNull() || 0 == last->length()) {
        qDebug() << "init music with empty playlist:" << last;
    }
    d->import->hide();
    d->titlebarwidget->setSearchEnable(true);
    d->newSonglistAction->setDisabled(false);
    d->musicList->raise();
    d->musicList->show();
    d->musicList->initData(last);
    d->musicList->onMusiclistChanged(last);
}

void MainWindow::initPlaylist(QList<PlaylistPtr > playlists, PlaylistPtr last)
{
    d->playlist->initData(playlists, last);
}

void MainWindow::initFooter(PlaylistPtr current, int mode)
{
    emit d->footer->initData(current, mode);
//    d->musicList->hide();
//    d->lyric->hide();
}

void MainWindow::binding(Presenter *presenter)
{
    connect(d->titlebarwidget, &TitleBarWidget::locateMusicInAllMusiclist,
            presenter, &Presenter::onLocateMusicAtAll);
    connect(d->titlebarwidget, &TitleBarWidget::search,
            presenter, &Presenter::onSearchText);
    connect(d->titlebarwidget, &TitleBarWidget::exitSearch,
            presenter, &Presenter::onExitSearch);

    connect(d->footer, &Footer::toggleLyricView, this, &MainWindow::toggleLyricView);
    connect(d->footer, &Footer::togglePlaylist, this, &MainWindow::togglePlaylist);

    connect(d->footer, &Footer::changeProgress, presenter, &Presenter::onChangeProgress);
    connect(d->footer, &Footer::locateMusic, presenter, &Presenter::locateMusic);
    connect(d->footer, &Footer::play, presenter, &Presenter::onMusicPlay);
    connect(d->footer, &Footer::resume, presenter, &Presenter::onMusicResume);
    connect(d->footer, &Footer::pause, presenter, &Presenter::onMusicPause);
    connect(d->footer, &Footer::next, presenter, &Presenter::onMusicNext);
    connect(d->footer, &Footer::prev, presenter, &Presenter::onMusicPrev);
    connect(d->footer, &Footer::toggleFavourite, presenter, &Presenter::onToggleFavourite);
    connect(d->footer, &Footer::modeChanged, presenter, &Presenter::onPlayModeChanged);
    connect(d->footer, &Footer::volumeChanged, presenter, &Presenter::onVolumeChanged);
    connect(d->footer, &Footer::toggleMute, presenter, &Presenter::onToggleMute);

    connect(presenter, &Presenter::coverSearchFinished,
            d->footer, &Footer::onCoverChanged);
    connect(presenter, &Presenter::musicPlayed,
            d->footer, &Footer::onMusicPlayed);
    connect(presenter, &Presenter::musicPaused,
            d->footer, &Footer::onMusicPause);
    connect(presenter, &Presenter::musicStoped,
            d->footer, &Footer::onMusicStoped);
    connect(presenter, &Presenter::musicAdded,
            d->footer, &Footer::onMusicAdded);
    connect(presenter, &Presenter::musiclistAdded,
            d->footer, &Footer::onMusicListAdded);
    connect(presenter, &Presenter::musicRemoved,
            d->footer, &Footer::onMusicRemoved);
    connect(presenter, &Presenter::progrossChanged,
            d->footer, &Footer::onProgressChanged);
    connect(presenter, &Presenter::volumeChanged,
            d->footer, &Footer::onVolumeChanged);
    connect(presenter, &Presenter::mutedChanged,
            d->footer, &Footer::onMutedChanged);

    connect(presenter, &Presenter::playlistResorted,
            d->musicList, &MusicListWidget::onMusiclistChanged);
    connect(presenter, &Presenter::currentPlaylistChanged,
            d->musicList, &MusicListWidget::onMusiclistChanged);
    connect(presenter, &Presenter::musicRemoved,
            d->musicList, &MusicListWidget::onMusicRemoved);
    connect(presenter, &Presenter::musiclistAdded,
            d->musicList, &MusicListWidget::onMusicListAdded);
    connect(presenter, &Presenter::musicPlayed,
            d->musicList, &MusicListWidget::onMusicPlayed);
    connect(presenter, &Presenter::musicPaused,
            d->musicList, &MusicListWidget::onMusicPause);
    connect(presenter, &Presenter::musicStoped,
            d->musicList, &MusicListWidget::onMusicPause);
    connect(presenter, &Presenter::musiclistMenuRequested,
            d->musicList, &MusicListWidget::onCustomContextMenuRequest);

    connect(d->musicList, &MusicListWidget::playall,
            presenter, &Presenter::onPlayall);
    connect(d->musicList, &MusicListWidget::resort,
            presenter, &Presenter::onResort);
    connect(d->musicList, &MusicListWidget::musicClicked,
            presenter, &Presenter::onSyncMusicPlay);
    connect(d->musicList, &MusicListWidget::requestCustomContextMenu,
            presenter, &Presenter::onRequestMusiclistMenu);
    connect(d->musicList, &MusicListWidget::addToPlaylist,
            presenter, &Presenter::onAddToPlaylist);
    connect(d->musicList, &MusicListWidget::musiclistRemove,
            presenter, &Presenter::onMusiclistRemove);
    connect(d->musicList, &MusicListWidget::musiclistDelete,
            presenter, &Presenter::onMusiclistDelete);

    connect(d->playlist, &PlaylistWidget::addPlaylist,
            presenter, &Presenter::onPlaylistAdd);
    connect(d->playlist, &PlaylistWidget::selectPlaylist,
            presenter, &Presenter::onSelectedPlaylistChanged);
    connect(d->playlist, &PlaylistWidget::playall,
            presenter, &Presenter::onPlayall);

    connect(presenter, &Presenter::currentPlaylistChanged,
            d->playlist, &PlaylistWidget::onCurrentChanged);
    connect(presenter, &Presenter::playlistAdded,
            d->playlist, &PlaylistWidget::onPlaylistAdded);
    connect(presenter, &Presenter::musicPlayed,
            d->playlist, &PlaylistWidget::onMusicPlayed);


    connect(d->lyricView, &LyricView::toggleLyricView, this, &MainWindow::toggleLyricView);
    connect(presenter, &Presenter::progrossChanged,
            d->lyricView, &LyricView::onProgressChanged);
    connect(presenter, &Presenter::musicPlayed,
            d->lyricView, &LyricView::onMusicPlayed);
    connect(presenter, &Presenter::lyricSearchFinished,
            d->lyricView, &LyricView::onLyricChanged);
    connect(presenter, &Presenter::coverSearchFinished,
            d->lyricView, &LyricView::onCoverChanged);
    connect(presenter, &Presenter::musicStoped,
            d->lyricView, &LyricView::onMusicStop);


    connect(d->playlist, &PlaylistWidget::hidePlaylist,
    this, [ = ]() {
        setPlaylistVisible(false);
    });

    connect(presenter, &Presenter::notifyMusciError,
    this, [ = ](PlaylistPtr playlist, const MusicMeta & meta, int error) {
        DDialog warnDlg(this);
        warnDlg.setIcon(QIcon(":/common/image/dialog_warning.png"));
        warnDlg.setTextFormat(Qt::RichText);
        warnDlg.setTitle(tr("File invalid or does not exist, load failed!"));
        warnDlg.addButtons(QStringList() << tr("I got it"));
        if (0 == warnDlg.exec()) {
            emit d->footer->play(playlist, meta);
        }
    });
    connect(presenter, &Presenter::musicError,
    this, [ = ](PlaylistPtr playlist, const MusicMeta & meta, int error) {
        d->musicList->onMusicError(playlist, meta, error);
    });

    connect(presenter, &Presenter::locateMusic,
    this, [ = ](PlaylistPtr playlist, const MusicMeta & info) {
        d->musicList->onLocate(playlist, info);
        d->playlist->onCurrentChanged(playlist);
        showMusicListView();
    });
    connect(presenter, &Presenter::notifyAddToPlaylist,
    this, [ = ](PlaylistPtr playlist, const MusicMetaList & /*metalist*/) {
        auto icon = QPixmap(":/common/image/notify_success.png");
        auto text =  tr("Successfully added to \"%1\"");
        text = text.arg(playlist->displayName());
        showTips(icon, text);
    });
    connect(presenter, &Presenter::currentPlaylistChanged,
    this, [ = ](PlaylistPtr playlist) {
        d->musicList->onMusiclistChanged(playlist);
        d->playlist->onCurrentChanged(playlist);
        showMusicListView();
    });
    connect(presenter, &Presenter::playlistAdded,
    this, [ = ](PlaylistPtr playlist) {
        qDebug() << playlist->id();
        setPlaylistVisible(true);
    });
    connect(presenter, &Presenter::coverSearchFinished,
    this, [ = ](const MusicMeta &, const QByteArray & coverData) {
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

        image = WidgetHelper::cropRect(image, this->size());
        setBackgroundImage(WidgetHelper::blurImage(image, 50));
        this->update();
    });
    connect(presenter, &Presenter::musicStoped,
    this, [ = ](PlaylistPtr, const MusicMeta &) {
        setCoverBackground(coverBackground());
    });
    connect(presenter, &Presenter::currentPlaylistChanged,
    this, [ = ]() {
        changeToMusicListView(false);
    });
    connect(presenter, &Presenter::requestImportFiles,
            this, &MainWindow::onSelectImportFiles);
    connect(presenter, &Presenter::meidaFilesImported,
    this, [ = ](PlaylistPtr playlist, MusicMetaList metalist) {
        DUtil::TimerSingleShot(1 * 800, [this, playlist, metalist ]() {
            this->showMusicListView();
        });
    });
    connect(presenter, &Presenter::metaInfoClean,
    this, [ = ]() {
        showImportView();
    });

    connect(d->import, &ImportWidget::scanMusicDirectory,
            presenter, &Presenter::onImportMusicDirectory);
    connect(d->import, &ImportWidget::importFiles,
            this, &MainWindow::onSelectImportFiles);
    connect(this, &MainWindow::importSelectFiles,
            presenter, &Presenter::onImportFiles);
    connect(this, &MainWindow::addPlaylist,
            presenter, &Presenter::onPlaylistAdd);

}

QString MainWindow::coverBackground() const
{
    return d->coverBackground;
}

void MainWindow::setCoverBackground(QString coverBackground)
{
    d->coverBackground = coverBackground;
    QImage image = QImage(coverBackground);
    image = WidgetHelper::cropRect(image, QWidget::size());
    setBackgroundImage(WidgetHelper::blurImage(image, 50));

    /////////////////
//    this->hide();
//    auto dsd = new DSettingDialog;
//    dsd->setFixedSize(720,580);
//    qDebug() << dsd;
//    dsd->show();
//    dsd->raise();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    ThinWindow::mousePressEvent(event);
    DUtil::TimerSingleShot(50, [this]() {
        setPlaylistVisible(false);
    });
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
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
            auto shortcut = DSettings::instance()->option(optkey).toStringList();
            auto modifiersstr = shortcut.value(0);
            auto scmodifiers = static_cast<Qt::KeyboardModifier>(modifiersstr.toInt());
            auto keystr = shortcut.value(1);
            auto sckey = static_cast<Qt::Key>(keystr.toInt());

            if (scmodifiers == keyModifiers && key == sckey && !ke->isAutoRepeat()) {
                qDebug() << "match " << optkey << ke->count() <<ke->isAutoRepeat();
                MusicApp::instance()->triggerShortcutAction(optkey);
                return true;
            }
        }


    }
    return qApp->eventFilter(obj, e);
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{

}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    ThinWindow::resizeEvent(e);
    QSize newSize = ThinWindow::size();

    d->titlebar->raise();
    d->titlebar->move(0, 0);

    d->lyricView->resize(newSize.width(), titleBarHeight);
    d->titlebarwidget->setFixedSize(newSize.width() - d->titlebar->buttonAreaWidth() - 10, titleBarHeight);

    d->lyricView->resize(newSize);
    d->musicList->setFixedSize(newSize);
    d->import->setFixedSize(newSize);

    d->playlist->setFixedSize(220, newSize.height() - footerHeight - titleBarHeight);

    d->footer->resize(newSize.width(), footerHeight);
    d->footer->raise();
    d->footer->show();
    d->footer->move(0, newSize.height() - footerHeight);

    if (d->tips) {
        d->tips->hide();
    }
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    ThinWindow::paintEvent(e);
}

void MainWindow::onCurrentPlaylistChanged(PlaylistPtr playlist)
{
    if (playlist->id() != SearchMusicListID) {
        d->titlebarwidget->exitSearch();
    }
}

void MainWindow::onSelectImportFiles()
{
    QFileDialog fileDlg;
    auto musicDir =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    fileDlg.setDirectory(musicDir.first());

    fileDlg.setViewMode(QFileDialog::Detail);
    fileDlg.setFileMode(QFileDialog::Directory);
    if (QFileDialog::Accepted == fileDlg.exec()) {
        d->import->showWaitHint();
        emit importSelectFiles(fileDlg.selectedFiles());
    }
}

void MainWindow::toggleLyricView()
{
    if (d->lyricView->isVisible()) {
        showMusicListView();
    } else {
        showLyricView();
    }
}

void MainWindow::togglePlaylist()
{
    setPlaylistVisible(!d->playlist->isVisible());
}

void MainWindow::showLyricView()
{
    auto current = d->currentWidget ? d->currentWidget : d->musicList;
    d->lyricView->resize(current->size());

    WidgetHelper::slideBottom2TopWidget(
        current, d->lyricView, s_AnimationDelay);

    this->disableControl();
    setPlaylistVisible(false);
    d->currentWidget = d->lyricView;
    d->titlebar->raise();
    d->footer->raise();

    updateViewname(s_PropertyViewnameLyric);
}

void MainWindow::showMusicListView()
{
    changeToMusicListView(false);

    updateViewname("");
}

void MainWindow::showImportView()
{
    if (d->import->isVisible()) {
        d->import->showImportHint();
        return;
    }

    setPlaylistVisible(false);
    auto current = d->currentWidget ? d->currentWidget : d->musicList;
    d->import->showImportHint();
    d->import->setFixedSize(current->size());

    qDebug() << "showImportView" << current << d->import;
    WidgetHelper::slideRight2LeftWidget(
        current, d->import, s_AnimationDelay);
    d->footer->enableControl(false);
    d->currentWidget = d->import;
    d->titlebar->raise();
    d->footer->raise();
    d->titlebarwidget->setSearchEnable(false);
    d->newSonglistAction->setDisabled(true);
    updateViewname("");
}

void MainWindow::showTips(QPixmap icon, QString text)
{
    if (d->tips) {
        d->tips->hide();
        d->tips->deleteLater();
    }
    d->tips = new Tip(icon, text , this);
    auto center = mapToGlobal(QPoint(QWidget::rect().center()));
    center.setY(center.y() + height() / 2 - d->footer->height() - 40 - 36);
    center = d->tips->mapFromGlobal(center);
    center = d->tips->mapToParent(center);
    d->tips->pop(center);
}

void MainWindow::setPlaylistVisible(bool visible)
{
    if (d->playlist->isVisible() == visible) {
        if (visible) {
            d->playlist->show();
            d->playlist->raise();
        }
        return;
    }

    QRect start(this->width(), titleBarHeight,
                d->playlist->width(), d->playlist->height());
    QRect end(this->width() - d->playlist->width() - 40, titleBarHeight,
              d->playlist->width(), d->playlist->height());
    if (!visible) {
        WidgetHelper::slideEdgeWidget(d->playlist, end, start, s_AnimationDelay, true);
    } else {
        WidgetHelper::slideEdgeWidget(d->playlist, start, end, s_AnimationDelay);
        d->playlist->setFocus();
        d->playlist->raise();
    }
    this->disableControl();
    d->titlebar->raise();
    d->footer->raise();
}

void MainWindow::changeToMusicListView(bool keepPlaylist)
{
    auto current = d->currentWidget ? d->currentWidget : d->import;
    qDebug() << "changeToMusicListView"
             << current << d->musicList << keepPlaylist;
    if (d->musicList->isVisible()) {
        d->musicList->raise();
        d->titlebar->raise();
        d->footer->raise();
        setPlaylistVisible(keepPlaylist);
        return;
    }
    d->musicList->setFixedSize(current->size());
    WidgetHelper::slideTop2BottomWidget(
        current, d->musicList, s_AnimationDelay);
    this->update();
    this->disableControl();
    d->currentWidget = d->musicList;
    setPlaylistVisible(keepPlaylist);
    d->titlebar->raise();
    d->footer->raise();

    d->titlebarwidget->setSearchEnable(true);
    d->newSonglistAction->setDisabled(false);
}

#include "widget/dsettingdialog.h"

void MainWindow::initMenu()
{
    d->newSonglistAction = new QAction(tr("New songlist"), this);
    connect(d->newSonglistAction, &QAction::triggered, this, [ = ](bool) {
        qDebug() << "" <<  d->newSonglistAction;
        setPlaylistVisible(true);
        emit this->addPlaylist(true);
    });

    auto m_addmusic = new QAction(tr("Add music"), this);
    connect(m_addmusic, &QAction::triggered, this, [ = ](bool) {
        this->onSelectImportFiles();
    });

    auto m_settings = new QAction(tr("Settings&&"), this);
    connect(m_settings, &QAction::triggered, this, [ = ](bool) {
        auto configDialog = new DSettingDialog(this);
        configDialog->setFixedSize(720, 580);
        DUtility::moveToCenter(configDialog);
        configDialog->show();
    });

    d->colorModeAction = new QAction(tr("Deep color mode"), this);
    d->colorModeAction->setCheckable(true);
    d->colorModeAction->setChecked(DSettings::instance()->option("base.play.theme").toString() == "dark");

    connect(d->colorModeAction, &QAction::triggered, this, [ = ](bool) {
        if (DThemeManager::instance()->theme() == "light") {
            d->colorModeAction->setChecked(true);
            DThemeManager::instance()->setTheme("dark");
            ThemeManager::instance()->setTheme("dark");
        } else {
            d->colorModeAction->setChecked(false);
            DThemeManager::instance()->setTheme("light");
            ThemeManager::instance()->setTheme("light");
        }
        DSettings::instance()->setOption("base.play.theme", DThemeManager::instance()->theme());
    });

    auto m_about = new QAction(tr("About"), this);
    connect(m_about, &QAction::triggered, this, [ = ](bool) {
        QString descriptionText = tr("Deepin Music Player is a beautiful design and "
                                     "simple function local music player. "
                                     "It supports viewing lyrics when playing, "
                                     "playing lossless music and creating customizable songlist, etc.");
        QString acknowledgementLink = "https://www.deepin.org/acknowledgments/deepin-music#thanks";

        auto *aboutDlg = new DAboutDialog(this);
        aboutDlg->setWindowIcon(QPixmap("::/common/image/logo.png"));
        aboutDlg->setProductIcon(QPixmap(":/common/image/logo_96.png"));
        aboutDlg->setProductName("Deepin Music");
        aboutDlg->setVersion(tr("Version: 3.0"));
        aboutDlg->setDescription(descriptionText);
        aboutDlg->setAcknowledgementLink(acknowledgementLink);
        aboutDlg->show();
    });

    QAction *m_help = new QAction(tr("Help"), this);
    connect(m_help, &QAction::triggered,
    this, [ = ](bool) {
        static QProcess *m_manual = nullptr;
        if (NULL == m_manual) {
            m_manual =  new QProcess(this);
            const QString pro = "dman";
            const QStringList args("deepin-music");
            connect(m_manual, static_cast<void(QProcess::*)(int)>(&QProcess::finished), this, [ = ](int) {
                m_manual->deleteLater();
                m_manual = nullptr;
            });
            m_manual->start(pro, args);
        }
    });

    QAction *m_close = new QAction(tr("Exit"), this);
    connect(m_close, &QAction::triggered, this, [ = ](bool) {
        this->close();
    });

    auto titleMenu = new QMenu;

    titleMenu->setStyle(QStyleFactory::create("dlight"));
    d->titlebar->setMenu(titleMenu);

    titleMenu->addAction(d->newSonglistAction);
    titleMenu->addAction(m_addmusic);
    titleMenu->addSeparator();

    titleMenu->addAction(d->colorModeAction);
    titleMenu->addAction(m_settings);
    titleMenu->addSeparator();

    titleMenu->addAction(m_about);
    titleMenu->addAction(m_help);
    titleMenu->addAction(m_close);
}

void MainWindow::disableControl()
{
    d->footer->enableControl(false);
    QTimer::singleShot(s_AnimationDelay, this, [ = ]() {
        d->footer->enableControl(true);
    });
}

void MainWindow::updateViewname(const QString &vm)
{
    DUtil::TimerSingleShot(s_AnimationDelay / 2, [this, vm]() {
        d->titlebar->setViewname(vm);
        d->titlebarwidget->setViewname(vm);
    });

}
