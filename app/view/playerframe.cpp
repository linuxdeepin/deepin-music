/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "playerframe.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QTimer>
#include <QProcess>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QFileDialog>
#include <QStandardPaths>

#include <dutility.h>
#include <dthememanager.h>
#include <DAboutDialog>

#include "titlebar.h"
#include "footer.h"
#include "importwidget.h"
#include "lyricview.h"
#include "playlistwidget.h"
#include "playlistview.h"
#include "playlistitem.h"
#include "musiclistwidget.h"

#include "../model/musiclistmodel.h"
#include "../core/playlist.h"
#include "../musicapp.h"
#include "musicitem.h"

DWIDGET_USE_NAMESPACE

class PlayerFramePrivate
{
public:
    ResizableStackedWidget  *stacked    = nullptr;
    QFrame          *content    = nullptr;
    TitleBar        *title      = nullptr;
    ImportWidget    *import     = nullptr;
    MusicListWidget *musicList  = nullptr;
    PlaylistWidget  *playlist   = nullptr;
    Footer          *footer     = nullptr;
    LyricView       *lyric      = nullptr;
};

static void slideWidget(QWidget *left, QWidget *right)
{
    right->show();
    left->show();
    right->resize(left->size());

    int delay = 500;
    QRect leftStart = QRect(0, 0, left->width(), left->height());
    QRect leftEnd = leftStart;
    leftEnd.moveLeft(-left->width());

    QPropertyAnimation *animation = new QPropertyAnimation(left, "geometry");
    animation->setDuration(delay);
    animation->setEasingCurve(QEasingCurve::InOutCubic);
    animation->setStartValue(leftStart);
    animation->setEndValue(leftEnd);
    animation->start();

    QRect rightStart = QRect(left->width(), 0, right->width(), right->height());
    QRect rightEnd = rightStart;
    rightEnd.moveRight(left->width() - 1);

    QPropertyAnimation *animation2 = new QPropertyAnimation(right, "geometry");
    animation2->setEasingCurve(QEasingCurve::InOutCubic);
    animation2->setDuration(delay);
    animation2->setStartValue(rightStart);
    animation2->setEndValue(rightEnd);
    animation2->start();

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        animation2, &QPropertyAnimation::deleteLater);
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        left, &QWidget::hide);

}

static void slideEdgeWidget(QWidget *right, QRect start, QRect end, bool hide = false)
{
    right->show();

    int delay = 200;

    QPropertyAnimation *animation2 = new QPropertyAnimation(right, "geometry");
    animation2->setEasingCurve(QEasingCurve::InCurve);
    animation2->setDuration(delay);
    animation2->setStartValue(start);
    animation2->setEndValue(end);
    animation2->start();
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        animation2, &QPropertyAnimation::deleteLater);
    if (hide)
        animation2->connect(animation2, &QPropertyAnimation::finished,
                            right, &QWidget::hide);

}

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE
#include <QMatrix3x3>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QGraphicsBlurEffect>
QImage applyEffectToImage(QImage src, QGraphicsEffect *effect, int extent = 0)
{
    if (src.isNull()) { return QImage(); }  //No need to do anything else!
    if (!effect) { return src; }            //No need to do anything else!
    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(src));
    item.setGraphicsEffect(effect);
    scene.addItem(&item);
    QImage res(src.size() + QSize(extent * 2, extent * 2), QImage::Format_ARGB32);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene.render(&ptr, QRectF(), QRectF(-extent, -extent, src.width() + extent * 2, src.height() + extent * 2));
    return res;
}

PlayerFrame::PlayerFrame(QWidget *parent)
    : DWindow(parent), d(new PlayerFramePrivate)
{
    QImage image = QImage((":/image/cover_max_background.png")).scaled(960, 720);
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(60);
    QImage result = applyEffectToImage(image, blur);

    setBackgroundImage(QPixmap::fromImage(result));
    setObjectName("PlayerFrame");

    d->title = new TitleBar;
    d->title->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setTitlebarWidget(d->title);

    d->content = new QFrame;
    d->content->setObjectName("BackgroundWidget");
    d->content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto contentLayout = new QVBoxLayout(d->content);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);

    d->import = new ImportWidget;
    d->musicList = new MusicListWidget;
    d->lyric = new LyricView;
    d->playlist = new PlaylistWidget(d->stacked);
    d->playlist->setFixedWidth(220);
    d->footer = new Footer;
    d->footer->setFixedHeight(60);

    d->stacked = new ResizableStackedWidget;
    d->stacked->setObjectName("ContentWidget");

    d->stacked->addWidget(d->import);
    d->stacked->addWidget(d->musicList);
    d->stacked->addWidget(d->lyric);
    d->stacked->addWidget(d->playlist);

    d->lyric->hide();
    d->import->hide();
    d->playlist->hide();
    d->musicList->hide();

    contentLayout->addWidget(d->stacked);
    contentLayout->addWidget(d->footer);
    setContentWidget(d->content);
    D_THEME_INIT_WIDGET(PlayerFrame);

    resize(960, 720);
}

PlayerFrame::~PlayerFrame()
{

}

void PlayerFrame::initMusiclist(QSharedPointer<Playlist> allmusic, QSharedPointer<Playlist> last)
{
    if (allmusic.isNull() || 0 == allmusic->length()) {
        d->import->show();
        d->musicList->hide();
        qWarning() << "no music in all music list" << allmusic;
        d->musicList->setCurrentList(allmusic);
        return;
    }

    if (last.isNull() || 0 == last->length()) {
        qDebug() << "init music with empty playlist:" << last;
    }
    d->import->hide();
    d->musicList->resize(958, 720 - 100);
    d->musicList->raise();
    d->musicList->show();
    d->musicList->setCurrentList(last);
    d->musicList->onMusiclistChanged(last);
}

void PlayerFrame::initPlaylist(QList<QSharedPointer<Playlist> > playlists, QSharedPointer<Playlist> last)
{
    d->playlist->initPlaylist(playlists, last);
}

void PlayerFrame::initFooter(QSharedPointer<Playlist> favlist, QSharedPointer<Playlist> current, int mode)
{
    emit d->footer->initFooter(favlist, current, mode);
}

void PlayerFrame::binding(AppPresenter *presenter)
{
    // Music list binding
    connect(presenter, &AppPresenter::selectedPlaylistChanged,
            d->musicList, &MusicListWidget::onMusiclistChanged);
    connect(presenter, &AppPresenter::musicRemoved,
            d->musicList, &MusicListWidget::onMusicRemoved);
    connect(presenter, &AppPresenter::musicAdded,
            d->musicList, &MusicListWidget::onMusicAdded);
    connect(presenter, &AppPresenter::musicPlayed,
            d->musicList, &MusicListWidget::onMusicPlayed);
    connect(presenter, &AppPresenter::musiclistMenuRequested,
            d->musicList, &MusicListWidget::onCustomContextMenuRequest);

    connect(d->musicList, &MusicListWidget::musicClicked,
            presenter, &AppPresenter::onMusicPlay);
    connect(d->musicList, &MusicListWidget::musicAdd,
            presenter, &AppPresenter::onMusicAdd);
    connect(d->musicList, &MusicListWidget::musicRemove,
            presenter, &AppPresenter::onMusicRemove);
    connect(d->musicList, &MusicListWidget::playall,
            presenter, &AppPresenter::onPlayall);
    connect(d->musicList, &MusicListWidget::requestCustomContextMenu,
            presenter, &AppPresenter::onRequestMusiclistMenu);

    // Play list bindding
    connect(presenter, &AppPresenter::selectedPlaylistChanged,
            d->playlist, &PlaylistWidget::onCurrentChanged);
    connect(presenter, &AppPresenter::playlistAdded,
            d->playlist, &PlaylistWidget::onPlaylistAdded);

    connect(d->playlist, &PlaylistWidget::addPlaylist,
            presenter, &AppPresenter::onPlaylistAdd);
    connect(d->playlist, &PlaylistWidget::selectPlaylist,
            presenter, &AppPresenter::onSelectedPlaylistChanged);

    // Footer Control
    connect(presenter, &AppPresenter::musicPlayed,
            d->footer, &Footer::onMusicPlay);
    connect(presenter, &AppPresenter::musicPaused,
            d->footer, &Footer::onMusicPause);
    connect(presenter, &AppPresenter::musicAdded,
            d->footer, &Footer::onMusicAdded);
    connect(presenter, &AppPresenter::musicRemoved,
            d->footer, &Footer::onMusicRemoved);
    connect(presenter, &AppPresenter::progrossChanged,
            d->footer, &Footer::onProgressChanged);

    connect(d->footer, &Footer::play,
            presenter, &AppPresenter::onMusicPlay);
    connect(d->footer, &Footer::pause,
            presenter, &AppPresenter::onMusicPause);
    connect(d->footer, &Footer::prev,
            presenter, &AppPresenter::onMusicPrev);
    connect(d->footer, &Footer::next,
            presenter, &AppPresenter::onMusicNext);
    connect(d->footer, &Footer::changeProgress,
            presenter, &AppPresenter::onChangeProgress);
    connect(d->footer, &Footer::toggleFavourite,
            presenter, &AppPresenter::onToggleFavourite);
    connect(d->footer, &Footer::modeChanged,
            presenter, &AppPresenter::onPlayModeChanged);

    // Import bindding
    connect(d->import, &ImportWidget::importMusicDirectory,
            presenter, &AppPresenter::onImportMusicDirectory);
    connect(d->import, &ImportWidget::importFiles,
            this, &PlayerFrame::onSelectImportFiles);
    connect(this, &PlayerFrame::importSelectFiles,
            presenter, &AppPresenter::onImportFiles);

    // View control
    connect(presenter, &AppPresenter::selectedPlaylistChanged,
    this, [ = ]() {
        if (d->lyric->isVisible()) {
            slideWidget(d->lyric, d->musicList);
            d->musicList->resize(d->lyric->size());
            d->musicList->show();
        }
    });

    connect(presenter, &AppPresenter::requestImportFiles,
            this, &PlayerFrame::onSelectImportFiles);

    connect(d->footer, &Footer::toggleLyric,
    this, [ = ]() {
        if (d->lyric->isVisible()) {
            slideWidget(d->lyric, d->musicList);
            d->musicList->resize(d->lyric->size());
            d->musicList->raise();
            d->musicList->show();
        } else {
            slideWidget(d->musicList, d->lyric);
            d->lyric->resize(d->musicList->size());
            d->lyric->raise();
            d->lyric->show();
        }

        // hide playlist
        if (d->playlist->isVisible())  {
            emit d->footer->togglePlaylist();
        }
    });

    connect(d->footer, &Footer::togglePlaylist,
    this, [ = ]() {
        d->playlist->resize(d->playlist->width(), d->stacked->height() - d->footer->height());
        QRect start(this->width(), 0, d->playlist->width(), d->playlist->height());
        QRect end(this->width() - d->playlist->width(), 0, d->playlist->width(), d->playlist->height());
        d->playlist->raise();
        if (d->playlist->isVisible()) {
            slideEdgeWidget(d->playlist, end, start, true);
        } else {
            slideEdgeWidget(d->playlist, start, end);
        }
    });

    connect(presenter, &AppPresenter::showPlaylist,
    this, [ = ]() {
        // show playlist
        if (!d->playlist->isVisible())  {
            emit d->footer->togglePlaylist();
        }
    });

    connect(presenter, &AppPresenter::showMusiclist,
    this, [ = ]() {
        if (d->import->isVisible()) {
            slideWidget(d->import, d->musicList);
        }
        d->musicList->resize(d->import->size());
        d->musicList->show();
    });
    initMenu();
}

void PlayerFrame::resizeEvent(QResizeEvent *e)
{
    DWindow::resizeEvent(e);
    QSize newSize = DWindow::size();
    d->stacked->setFixedSize(newSize.width(), newSize.height() - titlebarHeight());
    d->musicList->setFixedSize(newSize.width(),
                               newSize.height() - titlebarHeight() - d->footer->height() - 1);
    d->lyric->setFixedSize(newSize.width(),
                           newSize.height() - titlebarHeight() - d->footer->height() - 1);
    d->title->setFixedSize(newSize.width(), titlebarHeight() - 2);

    if (d->playlist->isVisible()) {
        d->playlist->resize(d->playlist->width(), d->stacked->height() - d->footer->height());
        QRect start(this->width(), 0, d->playlist->width(), d->playlist->height());
        QRect end(this->width() - d->playlist->width(), 0, d->playlist->width(), d->playlist->height());
        d->playlist->setGeometry(end);
    }
}

void PlayerFrame::paintEvent(QPaintEvent *e)
{
    int radius = 3;
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

    QPen borderPen(Qt::white);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(linearGradient));
    painter.strokePath(border, borderPen);
    painter.fillPath(border, QBrush(linearGradient));

    DWindow::paintEvent(e);
}

void PlayerFrame::onSelectImportFiles()
{
    QFileDialog fileDlg;
    auto musicDir =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    fileDlg.setDirectory(musicDir.first());

    fileDlg.setViewMode(QFileDialog::Detail);
    fileDlg.setFileMode(QFileDialog::Directory);
    if (QFileDialog::Accepted == fileDlg.exec()) {
        emit importSelectFiles(fileDlg.selectedFiles());
    }
}

void PlayerFrame::initMenu()
{

    auto m_newlist = new DAction(tr("New songlist"), this);
    connect(m_newlist, &DAction::triggered, this, [ = ](bool) {
        if (!d->playlist->isVisible()) {
            emit d->footer->togglePlaylist();
        }
        emit d->playlist->addPlaylist(true);
    });

    auto m_addmusic = new DAction(tr("Add music"), this);
    connect(m_addmusic, &DAction::triggered, this, [ = ](bool) {

        if (d->lyric->isVisible()) {
            slideWidget(d->lyric, d->musicList);
            d->musicList->resize(d->lyric->size());
            d->musicList->show();
            if (!d->playlist->isVisible()) {
                emit d->footer->togglePlaylist();
            }
        }
        this->onSelectImportFiles();
    });

    auto m_settings = new DAction(tr("Settings"), this);
    connect(m_settings, &DAction::triggered, this, [ = ](bool) {

    });

    auto m_colorMode = new DAction(tr("Deep color mode"), this);
    connect(m_colorMode, &DAction::triggered, this, [ = ](bool) {

    });

    auto m_about = new DAction(tr("About"), this);
    connect(m_about, &DAction::triggered, this, [ = ](bool) {
        QString descriptionText = tr("Deepin Music Player is a beautiful design and simple function local music player. It supports viewing lyrics when playing, playing lossless music and creating customizable songlist, etc.");
        DAboutDialog *about = new DAboutDialog(
            tr("Deepin Music"),
            QString(":/image/deepin-music.svg"),
            QString(":/image/about_icon.png"),
            tr("Deepin Music"),
            tr("Version: 3.0"),
            descriptionText,
            this);
        about->show();
    });

    DAction *m_help = new DAction(tr("Help"), this);
    connect(m_help, &DAction::triggered,
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

    DAction *m_close = new DAction(tr("Exit"), this);
    connect(m_close, &DAction::triggered, this, [ = ](bool) {
        this->close();
    });

    dbusMenu()->addAction(m_newlist);
    dbusMenu()->addAction(m_addmusic);
    dbusMenu()->addSeparator();

    dbusMenu()->addAction(m_colorMode);
    dbusMenu()->addAction(m_settings);
    dbusMenu()->addSeparator();

    dbusMenu()->addAction(m_about);
    dbusMenu()->addAction(m_help);
    dbusMenu()->addAction(m_close);
}
