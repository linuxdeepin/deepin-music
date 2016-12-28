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

#include <DUtil>
#include <dutility.h>
#include <DTitlebar>
#include <dthememanager.h>
#include <DAboutDialog>

#include "titlebar.h"
#include "footer.h"
#include "importwidget.h"
#include "lyricview.h"
#include "playlistwidget.h"
#include "musiclistwidget.h"

#include "../core/music.h"
#include "../core/playlist.h"
#include "../musicapp.h"
#include "widget/tip.h"
#include "../presenter/presenter.h"

#include "helper/widgethellper.h"

DWIDGET_USE_NAMESPACE

static const int s_AnimationDelay   = 350;
static QColor s_normalTitleTop      = QColor(255, 255, 255, 255);
static QColor s_normalTitleBottom   = QColor(0xf8, 0xf8, 0xf8, 255);
static QColor s_lyricTitleTop       = QColor(0, 0, 0, 94);
static QColor s_lyriclTitleBottom   = QColor(0, 0, 0, 102);
static const int titleBarHeight = 40;
static const int footerHeight = 60;

class MainWindowPrivate
{
public:
//    QStackedWidget  *stacked    = nullptr;
    QFrame          *content    = nullptr;
    DTitlebar       *title      = nullptr;
    TitleBar        *titlebar   = nullptr;
    Footer          *footer     = nullptr;
    PlaylistWidget  *playlist   = nullptr;

    ImportWidget    *import     = nullptr;
    MusicListWidget *musicList  = nullptr;
    LyricView       *lyricView      = nullptr;

    Tip             *tips       = nullptr;
    QWidget         *currentWidget = nullptr;
};


MainWindow::MainWindow(QWidget *parent)
    : ThinWindow(parent), d(new MainWindowPrivate)
{
    m_titlebarTopColor = s_normalTitleTop;
    m_titlebarBottomColor = s_normalTitleBottom;

    setFocusPolicy(Qt::ClickFocus);
    setObjectName("PlayerFrame");

    auto contentLayout = new QStackedLayout();
    setContentLayout(contentLayout);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);

    d->title = new DTitlebar();
    d->titlebar = new TitleBar();
    d->titlebar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    d->title->setFixedHeight(titleBarHeight);
    d->title->setCustomWidget(d->titlebar , Qt::AlignCenter, false);

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

    contentLayout->addWidget(d->title);

//    contentLayout->addWidget(d->titlebar);
    contentLayout->addWidget(d->import);
    contentLayout->addWidget(d->musicList);
    contentLayout->addWidget(d->lyricView);
    contentLayout->addWidget(d->playlist);
    contentLayout->addWidget(d->footer);

    D_THEME_INIT_WIDGET(MainWindow);

    d->footer->setFocus();

    connect(d->title, &DTitlebar::mouseMoving, this, &MainWindow::moveWindow);
    connect(d->footer, &Footer::mouseMoving, this, &MainWindow::moveWindow);
    setMinimumSize(840, 640);
    resize(QSize(840, 640));
}

MainWindow::~MainWindow()
{

}

void MainWindow::initMusiclist(PlaylistPtr allmusic, PlaylistPtr last)
{
    if (allmusic.isNull() || 0 == allmusic->length()) {
        d->import->show();
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
    connect(d->titlebar, &TitleBar::locateMusicInAllMusiclist,
            presenter, &Presenter::onLocateMusicAtAll);
    connect(d->titlebar, &TitleBar::search,
            presenter, &Presenter::onSearchText);
    connect(d->titlebar, &TitleBar::exitSearch,
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


    connect(d->lyricView, &LyricView::toggleLyricView, this, &MainWindow::toggleLyricView);
    connect(presenter, &Presenter::progrossChanged,
            d->lyricView, &LyricView::onProgressChanged);
    connect(presenter, &Presenter::musicPlayed,
            d->lyricView, &LyricView::onMusicPlayed);
    connect(presenter, &Presenter::lyricSearchFinished,
            d->lyricView, &LyricView::onLyricChanged);
    connect(presenter, &Presenter::coverSearchFinished,
            d->lyricView, &LyricView::onCoverChanged);


    connect(d->playlist, &PlaylistWidget::hidePlaylist,
    this, [ = ]() {
        setPlaylistVisible(false);
    });

    connect(presenter, &Presenter::notifyMusciError,
    this, [ = ](PlaylistPtr playlist, const MusicMeta & meta, int error) {
        DDialog warnDlg(this);
        warnDlg.setIcon(QIcon(":/light/image/diglog_warning.png"));
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
        auto icon = QPixmap(":/image/notify_success.png");
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
            return;
        }
        QImage image = QImage::fromData(coverData);
        if (image.isNull()) {
            return;
        }

        image = WidgetHelper::cropRect(image, this->size());
        setBackgroundImage(WidgetHelper::blurImage(image, 50));
        this->repaint();
    });
    connect(presenter, &Presenter::musicStoped,
    this, [ = ](PlaylistPtr, const MusicMeta &) {
        QImage image = QImage((":/image/cover_max.png"));
        image = WidgetHelper::cropRect(image, this->size());
        setBackgroundImage(WidgetHelper::blurImage(image, 50));
        repaint();
    });
    connect(presenter, &Presenter::currentPlaylistChanged,
    this, [ = ]() {
        changeToMusicListView(false);
    });
    connect(presenter, &Presenter::requestImportFiles,
            this, &MainWindow::onSelectImportFiles);
    connect(presenter, &Presenter::meidaFilesImported,
    this, [ = ](PlaylistPtr playlist, MusicMetaList metalist) {
        DUtil::TimerSingleShot(3 * 1000, [this, playlist, metalist ]() {
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

    initMenu();
}

void MainWindow::setDefaultBackground()
{
    QImage image = QImage((":/image/cover_max.png"));
    image = WidgetHelper::cropRect(image, QWidget::size());
    setBackgroundImage(WidgetHelper::blurImage(image, 50));
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    ThinWindow::mousePressEvent(event);
    setPlaylistVisible(false);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    ThinWindow::resizeEvent(e);
    QSize newSize = ThinWindow::size();

    d->title->raise();
    d->title->move(0, 0);

    d->lyricView->resize(newSize.width(), titleBarHeight);
    d->titlebar->setFixedSize(newSize.width() - d->title->buttonAreaWidth() - 10, titleBarHeight);

    d->lyricView->resize(newSize);
    d->musicList->resize(newSize);
    d->import->resize(newSize);

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
    return ThinWindow::paintEvent(e);

    {
        int radius = this->radius() ;
        int windowExtern = 40 + 1 * 2;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);

        QRect windowRect = QWidget::rect().marginsRemoved(
                               QMargins(windowExtern, windowExtern - 10, windowExtern, windowExtern + 10));

        QPoint topLeft(windowRect.x(), windowRect.y());
        QPoint bottomRight(windowRect.x() + windowRect.width(), windowRect.y() + windowRect.height());
        QPainterPath border;
        border.addRoundedRect(windowRect, radius, radius);

        QLinearGradient linearGradient(topLeft, QPoint(topLeft.x(), bottomRight.y()));
        linearGradient.setColorAt(0.0, Qt::white);
        linearGradient.setColorAt(0.2, Qt::white);
        linearGradient.setColorAt(1.0, Qt::white);

        QPen borderPen(QColor(0, 0, 0, 38));

        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QBrush(linearGradient));
        painter.strokePath(border, borderPen);
        painter.fillPath(border, QBrush(linearGradient));
    }

    ThinWindow::paintEvent(e);

    // draw header
    {
        return;
        int radius = 3 ;
        int windowExtern = 40 + 1 * 2;

        QPainter titlePainter(this);
        titlePainter.setRenderHint(QPainter::Antialiasing);
        titlePainter.setRenderHint(QPainter::HighQualityAntialiasing);

        QRect winRect = QWidget::rect().marginsRemoved(
                            QMargins(windowExtern, windowExtern - 10  , windowExtern, windowExtern + 10));

        QPoint titleTopLeft(winRect.x(), winRect.y());

        QRect topLeftRect(titleTopLeft,
                          QPoint(winRect.x() + 2 * radius, winRect.y() + 2 * radius));
        QRect topRightRect(QPoint(winRect.x() + winRect.width(), winRect.y()),
                           QPoint(winRect.x() + winRect.width() - 2 * radius,
                                  winRect.y() + 2 * radius));

        QPainterPath titleBorder;
        titleBorder.moveTo(winRect.x() + radius, winRect.y());
        titleBorder.lineTo(winRect.x() + winRect.width() - radius, winRect.y());
        titleBorder.arcTo(topRightRect, 90.0, 90.0);
        titleBorder.lineTo(winRect.x() + winRect.width(), winRect.y() + radius);
        titleBorder.lineTo(winRect.x() + winRect.width(), winRect.y() + 39);
        titleBorder.lineTo(winRect.x(), winRect.y() + 39);
        titleBorder.lineTo(winRect.x() , winRect.y() + radius);
        titleBorder.arcTo(topLeftRect, 90.0, 90.0);
        titleBorder.closeSubpath();

        QLinearGradient linearGradient(QPointF(0.0, 0.0), QPointF(0.0, 1.0));
        linearGradient.setColorAt(0.0, m_titlebarTopColor);
        linearGradient.setColorAt(1.0, m_titlebarBottomColor);

        QPen borderPen(QColor(255, 255, 255, 25));

        titlePainter.setBrush(QBrush(linearGradient));
        titlePainter.fillPath(titleBorder, QBrush(linearGradient));
        titlePainter.strokePath(titleBorder, borderPen);
        QLine line(titleTopLeft.x(), winRect.y() + 39,
                   winRect.x() + winRect.width(), winRect.y() + 39);

        titlePainter.setPen(QPen(QColor(0, 0, 0, 255 * 0.3), 0.5));
        titlePainter.drawLine(line);
    }
}

void MainWindow::onCurrentPlaylistChanged(PlaylistPtr playlist)
{
    if (playlist->id() != SearchMusicListID) {
        d->titlebar->exitSearch();
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
    m_titlebarTopColor = s_lyricTitleTop;
    m_titlebarBottomColor = s_lyriclTitleBottom;

    auto current = d->currentWidget ? d->currentWidget : d->musicList;
    d->lyricView->resize(current->size());
    WidgetHelper::slideBottom2TopWidget(
        current, d->lyricView, s_AnimationDelay);
    this->repaint();
    this->disableControl();
    setPlaylistVisible(false);
    d->currentWidget = d->lyricView;
}

void MainWindow::showMusicListView()
{
    changeToMusicListView(false);
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
        this->setFocus();
    } else {
        WidgetHelper::slideEdgeWidget(d->playlist, start, end, s_AnimationDelay);
        d->playlist->setFocus();
    }
    d->playlist->raise();
    this->disableControl();
    d->title->raise();
    d->footer->raise();
}

void MainWindow::changeToMusicListView(bool keepPlaylist)
{
    auto current = d->currentWidget ? d->currentWidget : d->import;
    qDebug() << "changeToMusicListView"
             << current << d->musicList << keepPlaylist;
    if (d->musicList->isVisible()) {
        d->musicList->raise();
        setPlaylistVisible(keepPlaylist);
        return;
    }

    m_titlebarTopColor = s_normalTitleTop;
    m_titlebarBottomColor = s_normalTitleBottom;

    d->musicList->setFixedSize(current->size());
    WidgetHelper::slideTop2BottomWidget(
        current, d->musicList, s_AnimationDelay);
    this->repaint();
    this->disableControl();
    d->currentWidget = d->musicList;
    setPlaylistVisible(keepPlaylist);
}

void MainWindow::initMenu()
{
    auto m_newlist = new QAction(tr("New songlist"), this);
    connect(m_newlist, &QAction::triggered, this, [ = ](bool) {
        setPlaylistVisible(true);
        emit this->addPlaylist(true);
    });

    auto m_addmusic = new QAction(tr("Add music"), this);
    connect(m_addmusic, &QAction::triggered, this, [ = ](bool) {
        showMusicListView();
        this->onSelectImportFiles();
    });

    auto m_settings = new QAction(tr("Settings"), this);
    connect(m_settings, &QAction::triggered, this, [ = ](bool) {

    });

    auto m_colorMode = new QAction(tr("Deep color mode"), this);
    connect(m_colorMode, &QAction::triggered, this, [ = ](bool) {

    });

    auto m_about = new QAction(tr("About"), this);
    connect(m_about, &QAction::triggered, this, [ = ](bool) {
        QString descriptionText = tr("Deepin Music Player is a beautiful design and "
                                     "simple function local music player. "
                                     "It supports viewing lyrics when playing, "
                                     "playing lossless music and creating customizable songlist, etc.");
        QString acknowledgementLink = "https://www.deepin.org/acknowledgments/deepin-music#thanks";

        auto *aboutDlg = new DAboutDialog(this);
        aboutDlg->setWindowIcon(QPixmap(":/image/deepin-music.svg"));
        aboutDlg->setProductIcon(QPixmap(":/image/about_icon.png"));
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
            const QStringList args("dde");
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

//    titleBarMenu()->addAction(m_newlist);
//    titleBarMenu()->addAction(m_addmusic);
//    titleBarMenu()->addSeparator();

//    titleBarMenu()->addAction(m_colorMode);
//    titleBarMenu()->addAction(m_settings);
//    titleBarMenu()->addSeparator();

//    titleBarMenu()->addAction(m_about);
//    titleBarMenu()->addAction(m_help);
//    titleBarMenu()->addAction(m_close);
}

void MainWindow::disableControl()
{
    d->footer->enableControl(false);
    QTimer::singleShot(s_AnimationDelay, this, [ = ]() {
        d->footer->enableControl(true);
    });
}
