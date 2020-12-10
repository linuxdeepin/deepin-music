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

#include "playlistwidget.h"

#include <QDebug>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMimeData>
#include <QResizeEvent>
#include <QStandardItemModel>

#include <DPushButton>
#include <DComboBox>
#include <DLabel>
#include <DPalette>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <QApplication>

#include "../core/music.h"
#include "../core/musicsettings.h"
#include "../core/util/inotifyfiles.h"
#include "../core/util/threadpool.h"
#include "playlistview.h"
#include "widget/ddropdown.h"
#include "../core/player.h"
#include "footerwidget.h"

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

static const int AnimationDelay = 400;

void PlayListWidget::initConntion()
{
//    connect(m_btClearAll, &DPushButton::clicked,
//    this, [ = ](bool) {
//        if (m_playListView->playlist()) {
//            Q_EMIT musiclistRemove(m_playListView->playlist(), m_playListView->playlist()->allmusic());
//        }
    //    });
}

void PlayListWidget::showAnimation(const QSize &size)
{
//    int height = 0;
//    int width = 0;
//    if (static_cast<QWidget *>(parent())) {
//        height = static_cast<QWidget *>(parent()->parent())->height();
//        width = static_cast<QWidget *>(parent()->parent())->width();
//    }

    rectAnimationSize = size;

    QRect start2(0, rectAnimationSize.height() - 85, rectAnimationSize.width(), 80);
    QRect end2(0, rectAnimationSize.height() - 450, rectAnimationSize.width(), 445);
    QPropertyAnimation *animation2 = new QPropertyAnimation(parent(), "geometry");
    animation2->setEasingCurve(QEasingCurve::InCurve);
    animation2->setDuration(AnimationDelay);
    animation2->setStartValue(start2);
    animation2->setEndValue(end2);
    animation2->start();
    this->show();
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        animation2, &QPropertyAnimation::deleteLater);
}

void PlayListWidget::closeAnimation(const QSize &size)
{
    rectAnimationSize = size;

    QRect start2(0, rectAnimationSize.height() - 85, rectAnimationSize.width(), 80);
    QRect end2(0, rectAnimationSize.height() - 450, rectAnimationSize.width(), 445);
    QPropertyAnimation *animation2 = new QPropertyAnimation(parent(), "geometry");
    animation2->setEasingCurve(QEasingCurve::InCurve);
    animation2->setDuration(AnimationDelay);
    animation2->setStartValue(end2);
    animation2->setEndValue(start2);
    animation2->start();
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        animation2, &QPropertyAnimation::deleteLater);
    animation2->connect(animation2, &QPropertyAnimation::finished, this, [ = ]() {
        this->hide();
        static_cast<FooterWidget *>(parent())->refreshBackground();
    });

    emit signalAutoHidden();
}

void PlayListWidget::slotPlayListChanged()
{
    m_playListView->playListChange();
}

//void PlayListWidget::showEmptyHits(bool empty)
//{
//    auto playlist = playListView->playlist();
//    if (playlist.isNull() || playlist->id() != SearchMusicListID) {
//        emptyHits->setText(PlayListWidget::tr("No songs"));
//    } else {
//        emptyHits->setText(PlayListWidget::tr("No search results"));
//    }
//    playListView->setVisible(!empty);
//    emptyHits->setVisible(empty);
//}

PlayListWidget::PlayListWidget(QWidget *parent) :
    DWidget(parent)
{
//    setFixedHeight(314);
//    this->setStyleSheet("background-color:red;");
    setAcceptDrops(true);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 30, 0, 0);
    layout->setSpacing(0);

    m_actionBar = new DWidget;
    m_actionBar->setFixedWidth(200);
    m_actionBar->setObjectName("PlayListActionBar");

    auto actionBarLayout = new QVBoxLayout(m_actionBar);
    actionBarLayout->setContentsMargins(36, 0, 8, 0);
    actionBarLayout->setSpacing(0);

    m_titleLabel = new DLabel();
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T3);
    m_titleLabel->setMargin(4);
    m_titleLabel->setText(tr("Play Queue"));
    m_titleLabel->setForegroundRole(DPalette::BrightText);

    m_infoLabel = new DLabel();
    DFontSizeManager::instance()->bind(m_infoLabel, DFontSizeManager::T6);
    m_infoLabel->setMargin(4);

    m_btClearAll = new DPushButton;
    m_btClearAll->setIcon(QIcon(":/mpimage/light/normal/clear_list_normal.svg"));
//    m_btClearAll->setFont(infoFont);
    DFontSizeManager::instance()->bind(m_btClearAll, DFontSizeManager::T6);
    m_btClearAll->setObjectName("PlayListPlayAll");
    m_btClearAll->setText(tr("Empty"));
    m_btClearAll->setFixedHeight(30);

    m_btClearAll->setFocusPolicy(Qt::TabFocus);
    m_btClearAll->setDefault(true);
//    m_btClearAll->installEventFilter(this);
//    this->installEventFilter(this);

    m_emptyHits = new DLabel(this);
    m_emptyHits->setObjectName("PlayListEmptyHits");
    m_emptyHits->hide();

    actionBarLayout->addWidget(m_titleLabel);
    actionBarLayout->addWidget(m_infoLabel);
    actionBarLayout->addWidget(m_btClearAll, 0, Qt::AlignLeft);
    actionBarLayout->addStretch();

    m_playListView = new PlayListView("play", true);
    m_playListView->show();
    m_playListView->setFocusPolicy(Qt::StrongFocus);
//    m_playListView->installEventFilter(this);

    layout->addWidget(m_actionBar, 0);
    layout->addWidget(m_playListView, 1);
    layout->addWidget(m_emptyHits, 0, Qt::AlignCenter);
    //show scroll bar
    layout->addSpacing(12);

    initConntion();

//    m_inotifyFiles.start();
//    ThreadPool::instance()->moveToNewThread(&m_inotifyFiles);

    connect(m_btClearAll, &DPushButton::clicked, this, &PlayListWidget::slotClearAllClicked);
    connect(m_playListView, &PlayListView::rowCountChanged, this, &PlayListWidget::slotUpdateItemCount);
    // 初始化播放列表数据
    this->slotPlayListChanged();
    connect(Player::instance(), &Player::signalPlayListChanged, this, &PlayListWidget::slotPlayListChanged);
    connect(qApp, &QApplication::focusChanged, this, &PlayListWidget::autoHidden);
    this->hide();
}

PlayListWidget::~PlayListWidget()
{
}

void PlayListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    DWidget::dragEnterEvent(event);
    if (event->mimeData()->hasFormat("text/uri-list")) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return;
    }
}

void PlayListWidget::dropEvent(QDropEvent *event)
{
    DWidget::dropEvent(event);

    if (!event->mimeData()->hasFormat("text/uri-list")) {
        return;
    }

    auto urls = event->mimeData()->urls();
    QStringList localpaths;
    for (auto &url : urls) {
        localpaths << url.toLocalFile();
    }

//    if (!localpaths.isEmpty() && !m_playListView->playlist().isNull()) {
//        Q_EMIT importSelectFiles(m_playListView->playlist(), localpaths);
//    }
}

void PlayListWidget::resizeEvent(QResizeEvent *event)
{
    //Q_D(PlayListWidget);
    DWidget::resizeEvent(event);
}

void PlayListWidget::slotTheme(int type)
{
    if (type == 0)
        type = DGuiApplicationHelper::instance()->themeType();
    if (type == 1) {
//        auto titleLabelPalette = d->infoLabel->palette();
//        titleLabelPalette.setColor(DPalette::ButtonText, ("#000000"));
//        d->titleLabel->setPalette(titleLabelPalette);
//        d->titleLabel->setForegroundRole(DPalette::ButtonText);

        auto infoLabelPalette = m_infoLabel->palette();
        QColor infoLabelColor = infoLabelPalette.color(DPalette::BrightText);
        infoLabelColor.setAlphaF(0.5);
        infoLabelPalette.setColor(DPalette::ButtonText, infoLabelColor);
        m_infoLabel->setPalette(infoLabelPalette);

        DPalette pl = m_btClearAll ->palette();
        pl.setColor(DPalette::ButtonText, QColor("#FFFFFF"));
        pl.setColor(DPalette::Light, QColor("#646464"));
        pl.setColor(DPalette::Dark, QColor("#5C5C5C"));
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.08);
        pl.setColor(DPalette::Shadow, sbcolor);
        m_btClearAll->setPalette(pl);
    } else {
//        auto titleLabelPalette = d->infoLabel->palette();
//        titleLabelPalette.setColor(DPalette::ButtonText, ("#FFFFFF"));
//        d->titleLabel->setPalette(titleLabelPalette);
//        d->titleLabel->setForegroundRole(DPalette::ButtonText);

        auto infoLabelPalette = m_infoLabel->palette();
        QColor infoLabelColor = infoLabelPalette.color(DPalette::BrightText);
        infoLabelColor.setAlphaF(0.5);
        infoLabelPalette.setColor(DPalette::ButtonText, infoLabelColor);
        m_infoLabel->setPalette(infoLabelPalette);

        DPalette pl = m_btClearAll->palette();
        pl.setColor(DPalette::ButtonText, QColor("#FFFFFF"));
        pl.setColor(DPalette::Light, QColor("#555454"));
        pl.setColor(DPalette::Dark, QColor("#414141"));
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.08);
        pl.setColor(DPalette::Shadow, sbcolor);
        m_btClearAll->setPalette(pl);
    }

    m_playListView->setThemeType(type);
}

void PlayListWidget::slotClearAllClicked()
{
    Player::instance()->clearPlayList();
    emit Player::instance()->signalPlayListChanged();
}

void PlayListWidget::slotUpdateItemCount()
{
    QString infoStr;
    int inum = m_playListView->model()->rowCount();
    if (inum == 0) {
        infoStr = tr("No songs");
    } else if (inum == 1) {
        infoStr = tr("1 song");
    } else {
        infoStr = tr("%1 songs").arg(inum);
    }
    m_infoLabel->setText(infoStr);
}

void PlayListWidget::autoHidden(QWidget *old, QWidget *now)
{
    if (old && now) {
        if (now->objectName() == "infoDialog" || now->objectName() == "InfoTitle" ||
                now->objectName() == "MainFrame" || now->objectName() == "MessageBox")  {
        } else {
            PlayListView *playListQueue = static_cast<PlayListView *>(now);
            if (playListQueue && playListQueue->objectName() == "PlayListView") {
                if (!playListQueue->getIsPlayQueue()) {
                    if (!this->isHidden())
                        closeAnimation(rectAnimationSize);
                }
            } else {
                if (now->objectName() != "FooterWidget") {
                    if (!this->isHidden())
                        closeAnimation(rectAnimationSize);
                }
            }
        }
    }
}

