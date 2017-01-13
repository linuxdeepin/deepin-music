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

#include "core/settings.h"
#include "../core/playlist.h"
#include "../musicapp.h"
#include "../presenter/presenter.h"
#include "helper/widgethellper.h"

DWIDGET_USE_NAMESPACE

static const int s_AnimationDelay   = 350 * 1;
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
    ImportWidget    *importWidget         = nullptr;
    MusicListWidget *musicList      = nullptr;
    LyricView       *lyricView      = nullptr;


    Tip             *tips           = nullptr;
    QWidget         *currentWidget  = nullptr;

    QAction         *newSonglistAction      = nullptr;
    QAction         *colorModeAction        = nullptr;
    QString         coverBackground         = ":/common/image/cover_max.png";
    QString         viewname                = "";
};

MainWindow::MainWindow(QWidget *parent)
    : ThinWindow(parent), d(new MainWindowPrivate)
{

    auto contentLayout = new QStackedLayout();
    setContentLayout(contentLayout);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);

//    connect(d->title, &DTitlebar::optionClicked, this, &MainWindow::optionClicked);

    d->importWidget = new ImportWidget;
    d->musicList = new MusicListWidget;
    d->lyricView = new LyricView;
    d->playlist = new PlaylistWidget;
    d->playlist->setFixedWidth(220);

    d->footer = new Footer;
    d->footer->setFixedHeight(footerHeight);

    d->musicList->setContentsMargins(0, titleBarHeight - 1, 0, footerHeight);
    d->lyricView->setContentsMargins(0, titleBarHeight - 1, 0, footerHeight);

    contentLayout->addWidget(d->titlebar);

//    contentLayout->addWidget(d->titlebar);
    contentLayout->addWidget(d->importWidget);
    contentLayout->addWidget(d->musicList);
    contentLayout->addWidget(d->lyricView);
    contentLayout->addWidget(d->playlist);
    contentLayout->addWidget(d->footer);

//    auto *bodyShadow = new QGraphicsDropShadowEffect;
//    bodyShadow->setBlurRadius(20.0);
//    bodyShadow->setColor(QColor(0, 0, 0, 0.10 * 255));
//    bodyShadow->setOffset(0, 5.0);
//    this->setGraphicsEffect(bodyShadow);


    d->footer->setFocus();

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
        d->importWidget->show();
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
    d->importWidget->hide();
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


    connect(d->footer, &Footer::toggleLyricView, this, &MainWindow::toggleLyricView);
    connect(d->footer, &Footer::togglePlaylist, this, &MainWindow::togglePlaylist);

    connect(d->footer, &Footer::changeProgress, presenter, &Presenter::onChangeProgress);
    connect(d->footer, &Footer::locateMusic, presenter, &Presenter::locateMusic);
    connect(d->footer, &Footer::play, presenter, &Presenter::onSyncMusicPlay);
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
    connect(presenter, &Presenter::musicListAdded,
            d->footer, &Footer::onMusicListAdded);
    connect(presenter, &Presenter::musicRemoved,
            d->footer, &Footer::onMusicRemoved);
    connect(presenter, &Presenter::progrossChanged,
            d->footer, &Footer::onProgressChanged);
    connect(presenter, &Presenter::volumeChanged,
            d->footer, &Footer::onVolumeChanged);
    connect(presenter, &Presenter::mutedChanged,
            d->footer, &Footer::onMutedChanged);


    connect(d->musicList, &MusicListWidget::updateMetaCodec,
            d->footer, &Footer::onUpdateMetaCodec);
    connect(d->musicList, &MusicListWidget::updateMetaCodec,
            d->lyricView, &LyricView::onUpdateMetaCodec);

    connect(presenter, &Presenter::musicListResorted,
            d->musicList, &MusicListWidget::onMusiclistChanged);
    connect(presenter, &Presenter::currentMusicListChanged,
            d->musicList, &MusicListWidget::onMusiclistChanged);
    connect(presenter, &Presenter::musicRemoved,
            d->musicList, &MusicListWidget::onMusicRemoved);
    connect(presenter, &Presenter::musicListAdded,
            d->musicList, &MusicListWidget::onMusicListAdded);
    connect(presenter, &Presenter::musicPlayed,
            d->musicList, &MusicListWidget::onMusicPlayed);
    connect(presenter, &Presenter::musicPaused,
            d->musicList, &MusicListWidget::onMusicPause);
    connect(presenter, &Presenter::musicStoped,
            d->musicList, &MusicListWidget::onMusicPause);
    connect(presenter, &Presenter::requestMusicListMenu,
            d->musicList, &MusicListWidget::onCustomContextMenuRequest);

    connect(d->musicList, &MusicListWidget::updateMetaCodec,
            presenter, &Presenter::onUpdateMetaCodec);
    connect(d->musicList, &MusicListWidget::playall,
            presenter, &Presenter::onPlayall);
    connect(d->musicList, &MusicListWidget::resort,
            presenter, &Presenter::onResort);
    connect(d->musicList, &MusicListWidget::playMedia,
            presenter, &Presenter::onSyncMusicPlay);
    connect(d->musicList, &MusicListWidget::requestCustomContextMenu,
            presenter, &Presenter::onRequestMusiclistMenu);
    connect(d->musicList, &MusicListWidget::addToPlaylist,
            presenter, &Presenter::onAddToPlaylist);
    connect(d->musicList, &MusicListWidget::musiclistRemove,
            presenter, &Presenter::onMusiclistRemove);
    connect(d->musicList, &MusicListWidget::musiclistDelete,
            presenter, &Presenter::onMusiclistDelete);
    connect(d->musicList, &MusicListWidget::importSelectFiles,
            presenter, &Presenter::requestImportPaths);

    connect(d->playlist, &PlaylistWidget::addPlaylist,
            presenter, &Presenter::onPlaylistAdd);
    connect(d->playlist, &PlaylistWidget::selectPlaylist,
            presenter, &Presenter::onCurrentPlaylistChanged);
    connect(d->playlist, &PlaylistWidget::playall,
            presenter, &Presenter::onPlayall);

    connect(presenter, &Presenter::currentMusicListChanged,
            d->playlist, &PlaylistWidget::onCurrentChanged);
    connect(presenter, &Presenter::playlistAdded,
            d->playlist, &PlaylistWidget::onPlaylistAdded);
    connect(presenter, &Presenter::musicPlayed,
            d->playlist, &PlaylistWidget::onMusicPlayed);


    connect(d->lyricView, &LyricView::toggleLyricView, this, &MainWindow::toggleLyricView);

    connect(d->lyricView, &LyricView::requestContextSearch,
            presenter, &Presenter::requestContextSearch);
    connect(d->lyricView, &LyricView::changeMetaCache,
            presenter, &Presenter::onChangeSearchMetaCache);
    connect(presenter, &Presenter::contextSearchFinished,
            d->lyricView, &LyricView::contextSearchFinished);
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

        QFont font(this->font());
        QFontMetrics fm(font);
        auto displayName = fm.elidedText(playlist->displayName(), Qt::ElideMiddle, 300);
        auto text =  tr("Successfully added to \"%1\"").arg(displayName);
        showTips(icon, text);
    });
    connect(presenter, &Presenter::currentMusicListChanged,
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
    connect(presenter, &Presenter::currentMusicListChanged,
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
    connect(presenter, &Presenter::metaLibraryClean,
    this, [ = ]() {
        showImportView();
    });

    connect(this, &MainWindow::importSelectFiles,
            presenter, &Presenter::onImportFiles);
    connect(this, &MainWindow::addPlaylist,
            presenter, &Presenter::onPlaylistAdd);


}



QString MainWindow::viewname() const
{
    return d->viewname;
}



void MainWindow::mousePressEvent(QMouseEvent *event)
{
    ThinWindow::mousePressEvent(event);
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
                qDebug() << "match " << optkey << ke->count() << ke->isAutoRepeat();
                MusicApp::instance()->triggerShortcutAction(optkey);
                return true;
            }
        }
    }
    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
//        qDebug() << obj << me->pos();
        if (obj->objectName() == this->objectName() || this->objectName() + "Window" == obj->objectName()) {
//            qDebug() << me->pos() << QCursor::pos();
            QPoint mousePos = me->pos();
            auto geometry = d->playlist->geometry().marginsAdded(QMargins(0, 0, 40, 40));
//            qDebug() << geometry << mousePos;
            if (!geometry.contains(mousePos)) {
                qDebug() << "hide playlist" << me->pos() << QCursor::pos() << obj;
                DUtil::TimerSingleShot(50, [this]() {
                    setPlaylistVisible(false);
                });
            }
        }
    }

    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        //        qDebug() << obj << me->pos();
        if (obj->objectName() == this->objectName() || this->objectName() + "Window" == obj->objectName()) {
            QPoint mousePos = me->pos();
            qDebug() << "lyricView checkHiddenSearch" << me->pos() << QCursor::pos() << obj;
            d->lyricView->checkHiddenSearch(mousePos);
        }

    }

    if (e->type() == QEvent::Close) {
        if (obj->objectName() == this->objectName()) {
            exit(0);
        }
    }
    return qApp->eventFilter(obj, e);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    ThinWindow::resizeEvent(e);
    QSize newSize = ThinWindow::size();

    d->titlebar->raise();
    d->titlebar->move(0, 1);

    d->titlebarwidget->setFixedSize(newSize.width() - d->titlebar->buttonAreaWidth() - 10, titleBarHeight);

    d->lyricView->resize(newSize);
    d->musicList->setFixedSize(newSize);
    d->importWidget->setFixedSize(newSize);

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
//    d->titlebar->hide();
    ThinWindow::paintEvent(e);
}

void MainWindow::onCurrentPlaylistChanged(PlaylistPtr playlist)
{
    if (playlist->id() != SearchMusicListID) {
        d->titlebarwidget->exitSearch();
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
//    d->lyricView->resize(current->size());

    WidgetHelper::slideBottom2TopWidget(
        current,  d->lyricView, s_AnimationDelay);

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
    if (d->importWidget->isVisible()) {
        d->importWidget->showImportHint();
        return;
    }

    setPlaylistVisible(false);
    auto current = d->currentWidget ? d->currentWidget : d->musicList;
    d->importWidget->showImportHint();
    d->importWidget->setFixedSize(current->size());

    qDebug() << "showImportView" << current << d->importWidget;
    WidgetHelper::slideRight2LeftWidget(
        current, d->importWidget, s_AnimationDelay);
    d->footer->enableControl(false);
    d->currentWidget = d->importWidget;
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
            d->playlist->setFocus();
            d->playlist->show();
            d->playlist->raise();
        }
        return;
    }

    double factor = 0.6;
    QRect start(this->width(), titleBarHeight,
                d->playlist->width(), d->playlist->height());
    QRect end(this->width() - d->playlist->width() - this->shadowWidth() * 2, titleBarHeight,
              d->playlist->width(), d->playlist->height());
    if (!visible) {
        WidgetHelper::slideEdgeWidget(d->playlist, end, start, s_AnimationDelay * factor, true);
        d->footer->setFocus();
    } else {
        d->playlist->setFocus();
        WidgetHelper::slideEdgeWidget(d->playlist, start, end, s_AnimationDelay * factor);
        d->playlist->raise();
    }
    this->disableControl(s_AnimationDelay * factor);
    d->titlebar->raise();
    d->footer->raise();
}

void MainWindow::setViewname(QString viewname)
{
    if (d->viewname == viewname) {
        return;
    }

    d->viewname = viewname;
    emit viewnameChanged(viewname);
}

void MainWindow::changeToMusicListView(bool keepPlaylist)
{
    auto current = d->currentWidget ? d->currentWidget : d->importWidget;
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

void MainWindow::initMenu()
{

}

void MainWindow::disableControl(int delay)
{
    d->footer->enableControl(false);
    QTimer::singleShot(s_AnimationDelay, this, [ = ]() {
        d->footer->enableControl(true);
    });
}

void MainWindow::updateViewname(const QString &vm)
{
    DUtil::TimerSingleShot(s_AnimationDelay / 2, [this, vm]() {
        this->setViewname(vm);
        d->titlebar->setViewname(vm);
        d->titlebarwidget->setViewname(vm);
    });

}
