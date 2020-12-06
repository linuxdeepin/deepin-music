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

#include "../core/music.h"
#include "../core/musicsettings.h"
#include "../core/util/inotifyfiles.h"
#include "../core/util/threadpool.h"
#include "playlistview.h"
#include "widget/ddropdown.h"
#include "../core/player.h"

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

void PlayListWidget::initConntion()
{
//    connect(m_btClearAll, &DPushButton::clicked,
//    this, [ = ](bool) {
//        if (m_playListView->playlist()) {
//            Q_EMIT musiclistRemove(m_playListView->playlist(), m_playListView->playlist()->allmusic());
//        }
//    });
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
    m_btClearAll->installEventFilter(this);
    this->installEventFilter(this);

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
    m_playListView->installEventFilter(this);

    layout->addWidget(m_actionBar, 0);
    layout->addWidget(m_playListView, 1);
    layout->addWidget(m_emptyHits, 0, Qt::AlignCenter);
    //show scroll bar
    layout->addSpacing(12);

    initConntion();

    int themeType = DGuiApplicationHelper::instance()->themeType();
    slotTheme(themeType);

//    m_inotifyFiles.start();
//    ThreadPool::instance()->moveToNewThread(&m_inotifyFiles);

    connect(m_btClearAll, &DPushButton::clicked, this, &PlayListWidget::slotClearAllClicked);
}

PlayListWidget::~PlayListWidget()
{
}

bool PlayListWidget::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *event = static_cast<QKeyEvent *>(e);
        if (event->key() == Qt::Key_Escape) {

            Q_EMIT btPlayList();
        }
    }

    if (o == m_playListView) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *event = static_cast<QKeyEvent *>(e);
            if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_M)) {

                int rowIndex = m_playListView->currentIndex().row();
                int row = 40 * rowIndex;
                QPoint pos;

                if (row > 300) {
                    QPoint posm(300, 65);
                    pos = posm;
                } else {
                    QPoint posm(300, row);
                    pos = posm;
                }

                Q_EMIT requestCustomContextMenu(pos, 0);

            } else if (event->key() == Qt::Key_Escape) {

            }
        } else if (e->type() == QEvent::FocusIn) {

            int rowIndex = m_playListView->currentIndex().row();
            if (rowIndex == -1) {
                auto index = m_playListView->item(0, 0);
                m_playListView->setCurrentItem(index);
            }

        } else if (e->type() == QEvent::FocusOut) {

            int rowIndex = m_playListView->currentIndex().row();
            if (rowIndex == 0) {
                //    d->playListView->clearSelection();
            }
        }
    } else if (o == m_btClearAll) {

        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *event = static_cast<QKeyEvent *>(e);
            if (event->key() == Qt::Key_Escape) {

            }
        } else  if (e->type() == QEvent::FocusIn) {
            auto index = m_playListView->item(-1, 0);
            m_playListView->setCurrentItem(index);

        } else if (e->type() == QEvent::FocusOut) {

        }
    }

    return QWidget::eventFilter(o, e);
}

void PlayListWidget::showEvent(QShowEvent *event)
{
    qDebug() << "zy------PlayListWidget::showEvent width = " << width();
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

void PlayListWidget::changePicture(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap)
{
//    m_playListView->setPlayPixmap(pixmap, sidebarPixmap, albumPixmap);
}

void PlayListWidget::slotClearAllClicked()
{
    Player::instance()->clearPlayList();
    emit Player::instance()->signalPlayListChanged();
}

