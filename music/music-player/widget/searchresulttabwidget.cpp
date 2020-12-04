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

#include "searchresulttabwidget.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QApplication>
#include <QFocusEvent>
#include <QGraphicsOpacityEffect>

#include <DWidgetUtil>
#include <DPushButton>
#include <DLabel>
#include <DHorizontalLine>
#include <DFontSizeManager>

#include "label.h"
#include "linewidget.h"
#include "playlistview.h"
#include "albumlistview.h"
#include "singerlistview.h"

#include "ac-desktop-define.h"

DWIDGET_USE_NAMESPACE


SearchResultTabWidget::SearchResultTabWidget(QWidget *parent) :
    DWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setObjectName("SearchResultTabWidget");

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(0, 14, 0, 0);
    vlayout->setSpacing(8);
    setLayout(vlayout);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(17, 0, 0, 0);
    vlayout->addLayout(hLayout);

    //音乐
    m_music = new Label(tr("歌曲"), this);
    m_music->setFixedSize(42, 20);
    m_music->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_music, DFontSizeManager::T6);
    m_music->setEnabled(false);
    hLayout->addWidget(m_music);
    //歌手
    m_singer = new Label(tr("演唱者"), this);
    m_singer->setFixedSize(42, 20);
    m_singer->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_singer, DFontSizeManager::T6);
    m_singer->setEnabled(false);
    hLayout->addWidget(m_singer);
    //专辑
    m_album = new Label(tr("专辑"), this);
    m_album->setFixedSize(42, 20);
    m_album->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_album, DFontSizeManager::T6);
    m_album->setEnabled(false);
    hLayout->addWidget(m_album);
    hLayout->addStretch();

    //分割线1
    m_Line = new DHorizontalLine;
    m_Line->setFrameShadow(DHorizontalLine::Raised);
    m_Line->setLineWidth(1);
    m_Line->lower();

    QHBoxLayout *hLineLayout = new QHBoxLayout();
    hLineLayout->setContentsMargins(17, 0, 12, 0);
    hLineLayout->addWidget(m_Line);
    vlayout->addLayout(hLineLayout);

    m_StackedWidget = new QStackedWidget(this);
    vlayout->addWidget(m_StackedWidget);

    m_musicListView = new PlayListView("musicResult", false);
    m_StackedWidget->addWidget(m_musicListView);

    m_singerListView = new SingerListView("artistResult", this);
    m_StackedWidget->addWidget(m_singerListView);

    m_albumListView = new AlbumListView("albumResult", this);
    m_StackedWidget->addWidget(m_albumListView);

    m_LineWidget = new LineWidget(this);
    m_LineWidget->setFixedSize(42, 2);
    m_LineWidget->hide();
}

SearchResultTabWidget::~SearchResultTabWidget()
{
}

void SearchResultTabWidget::refreshListview(ListPageSwitchType switchtype, QString searchword)
{
    m_switchtype = switchtype;
    m_searchWord = searchword;
    if (switchtype == SearchMusicResultType) {
        setLabelChecked(m_music);
        m_StackedWidget->setCurrentWidget(m_musicListView);
        m_musicListView->resetSonglistByStr(searchword);
        m_singerListView->resetSingerListDataBySongName(m_musicListView->getSonglistInListview());
        m_albumListView->resetAlbumListDataBySongName(m_musicListView->getSonglistInListview());
    } else if (switchtype == SearchSingerResultType) {
        setLabelChecked(m_singer);
        m_StackedWidget->setCurrentWidget(m_singerListView);
        m_singerListView->resetSingerListDataByStr(searchword);
        m_musicListView->resetSonglistBySinger(m_singerListView->getSingerListData());
        m_albumListView->resetAlbumListDataBySinger(m_singerListView->getSingerListData());
    } else if (switchtype == SearchAlbumResultType) {
        setLabelChecked(m_album);
        m_StackedWidget->setCurrentWidget(m_albumListView);
        m_albumListView->resetAlbumListDataByStr(searchword);
        m_musicListView->resetSonglistByAlbum(m_albumListView->getAlbumListData());
        m_singerListView->resetSingerListDataByAlbum(m_albumListView->getAlbumListData());
    }
}

void SearchResultTabWidget::setViewMode(QListView::ViewMode mode)
{
    if (m_StackedWidget->currentWidget() == m_musicListView) {
        m_musicListView->setViewModeFlag("musicResult", mode);
    } else if (m_StackedWidget->currentWidget() == m_albumListView) {
        m_albumListView->setViewMode(mode);
    } else if (m_StackedWidget->currentWidget() == m_singerListView) {
        m_singerListView->setViewMode(mode);
    }
}

void SearchResultTabWidget::showEvent(QShowEvent *event)
{
    m_musicRect = m_music->rect();
    Q_UNUSED(event)
}

void SearchResultTabWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
}

void SearchResultTabWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_music->geometry().contains(event->pos())) {
        qDebug() << "zy------m_music clicked";
        setLabelChecked(m_music);
    } else if (m_singer->geometry().contains(event->pos())) {
        qDebug() << "zy------m_singer clicked";
        setLabelChecked(m_singer);
    } else if (m_album->geometry().contains(event->pos())) {
        qDebug() << "zy------m_album clicked";
        setLabelChecked(m_album);
    }
    DWidget::mousePressEvent(event);
}

void SearchResultTabWidget::setLabelChecked(Label *label)
{
    if (m_music == label) {
        m_singer->setEnabled(false);
        m_album->setEnabled(false);
        m_music->setEnabled(true);
        m_LineWidget->show();
        m_LineWidget->move(m_music->x(), m_music->y() + m_music->height() + 8);
        m_StackedWidget->setCurrentWidget(m_musicListView);
    } else if (m_singer == label) {
        m_singer->setEnabled(true);
        m_album->setEnabled(false);
        m_music->setEnabled(false);
        m_LineWidget->show();
        m_LineWidget->move(m_singer->x(), m_singer->y() + m_singer->height() + 8);
        m_StackedWidget->setCurrentWidget(m_singerListView);
    } else if (m_album == label) {
        m_singer->setEnabled(false);
        m_album->setEnabled(true);
        m_music->setEnabled(false);
        m_LineWidget->show();
        m_LineWidget->move(m_album->x(), m_album->y() + m_album->height() + 8);
        m_StackedWidget->setCurrentWidget(m_albumListView);
    }
}
