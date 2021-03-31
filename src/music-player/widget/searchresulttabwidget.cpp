/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
#include "speechCenter.h"
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
    hLayout->setSpacing(25);
    vlayout->addLayout(hLayout);

    //音乐
    m_music = new Label(tr("Songs"), this);
    m_music->resize(static_cast<int>(strlen(reinterpret_cast<char *>(m_music->text().data()))) * m_music->font().pointSize(), 20);
    m_music->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_music, DFontSizeManager::T6);
    m_music->setEnabled(false);
    hLayout->addWidget(m_music);
    //歌手
    m_singer = new Label(tr("Artists"), this);
    // 根据字体大小重置控件大小
    m_singer->resize(static_cast<int>(strlen(reinterpret_cast<char *>(m_singer->text().data()))) * m_singer->font().pointSize(), 20);
    m_singer->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_singer, DFontSizeManager::T6);
    m_singer->setEnabled(false);
    hLayout->addWidget(m_singer);
    //专辑
    m_album = new Label(tr("Albums"), this);
    m_album->resize(static_cast<int>(strlen(reinterpret_cast<char *>(m_album->text().data()))) * m_album->font().pointSize(), 20);
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
    m_singerListView->setViewModeFlag(m_singerListView->getViewMode());
    m_StackedWidget->addWidget(m_singerListView);

    m_albumListView = new AlbumListView("albumResult", this);
    m_albumListView->setViewModeFlag(m_albumListView->getViewMode());
    m_StackedWidget->addWidget(m_albumListView);

    m_LineWidget = new LineWidget(this);
    m_LineWidget->setFixedSize(43, 2);
    m_LineWidget->hide();

    connect(m_musicListView, &PlayListView::musicResultListCountChanged,
            this, &SearchResultTabWidget::sigSearchTypeChanged);
    this->connect(m_music, &Label::signalSizeChange, this, &SearchResultTabWidget::slotSizeChange);
}

SearchResultTabWidget::~SearchResultTabWidget()
{
}

void SearchResultTabWidget::refreshListview(ListPageSwitchType switchtype, const QString &searchword)
{
    m_switchtype = switchtype;
    m_searchWord = searchword;
    refreshListview();
}

void SearchResultTabWidget::refreshListview()
{
    if (m_switchtype == SearchMusicResultType) {
        setLabelChecked(m_music);
        m_StackedWidget->setCurrentWidget(m_musicListView);
        m_musicListView->resetSonglistByStr(m_searchWord);
        m_singerListView->resetSingerListDataBySongName(m_musicListView->getMusicListData());
        m_albumListView->resetAlbumListDataBySongName(m_musicListView->getMusicListData());
        emit sigSearchTypeChanged("musicResult");
    } else if (m_switchtype == SearchSingerResultType) {
        setLabelChecked(m_singer);
        m_StackedWidget->setCurrentWidget(m_singerListView);
        m_singerListView->resetSingerListDataByStr(m_searchWord);
        m_musicListView->resetSonglistBySinger(m_singerListView->getSingerListData());
        m_albumListView->resetAlbumListDataBySinger(m_singerListView->getSingerListData());
        emit sigSearchTypeChanged("artistResult");
        qDebug() << "artistResult";
    } else if (m_switchtype == SearchAlbumResultType) {
        setLabelChecked(m_album);
        m_StackedWidget->setCurrentWidget(m_albumListView);
        m_albumListView->resetAlbumListDataByStr(m_searchWord);
        m_musicListView->resetSonglistByAlbum(m_albumListView->getAlbumListData());
        m_singerListView->resetSingerListDataByAlbum(m_albumListView->getAlbumListData());
        emit sigSearchTypeChanged("albumResult");
        qDebug() << "albumResult";
    }
}

void SearchResultTabWidget::setViewMode(QListView::ViewMode mode)
{
    if (m_StackedWidget->currentWidget() == m_musicListView) {
        m_musicListView->setViewModeFlag("musicResult", mode);
    } else if (m_StackedWidget->currentWidget() == m_albumListView) {
        m_albumListView->setViewModeFlag(mode);
    } else if (m_StackedWidget->currentWidget() == m_singerListView) {
        m_singerListView->setViewModeFlag(mode);
    }
}

void SearchResultTabWidget::setSortType(DataBaseService::ListSortType sortType)
{
    if (m_StackedWidget->currentWidget() == m_musicListView) {
        m_musicListView->setSortType(sortType);
    } else if (m_StackedWidget->currentWidget() == m_albumListView) {
        m_albumListView->setSortType(sortType);
    } else if (m_StackedWidget->currentWidget() == m_singerListView) {
        m_singerListView->setSortType(sortType);
    }
}

DataBaseService::ListSortType SearchResultTabWidget::getSortType()
{
    if (m_StackedWidget->currentWidget() == m_musicListView) {
        return m_musicListView->getSortType();
    } else if (m_StackedWidget->currentWidget() == m_albumListView) {
        return m_albumListView->getSortType();
    } else if (m_StackedWidget->currentWidget() == m_singerListView) {
        return m_singerListView->getSortType();
    }
    return DataBaseService::SortByNull;
}

QListView::ViewMode SearchResultTabWidget::getViewMode()
{
    if (m_StackedWidget->currentWidget() == m_musicListView) {
        return m_musicListView->getViewMode();
    } else if (m_StackedWidget->currentWidget() == m_albumListView) {
        return m_albumListView->getViewMode();
    } else if (m_StackedWidget->currentWidget() == m_singerListView) {
        return m_singerListView->getViewMode();
    }
    return QListView::IconMode;
}

int SearchResultTabWidget::getMusicCountByMusic()
{
    int count = 0;
    if (m_musicListView) {
        count = m_musicListView->count();
    }
    return count;
}

int SearchResultTabWidget::getAlbumCount()
{
    int count = 0;
    if (m_albumListView) {
        count = m_albumListView->getAlbumListData().size();
    }
    return count;
}

int SearchResultTabWidget::getMusicCountByAlbum()
{
    return m_albumListView->getMusicCount();
}

int SearchResultTabWidget::getSingerCount()
{
    int count = 0;
    if (m_singerListView) {
        count = m_singerListView->getSingerListData().size();
    }
    return count;
}

int SearchResultTabWidget::getMusicCountBySinger()
{
    return m_singerListView->getMusicCount();
}

QList<MediaMeta> SearchResultTabWidget::getMusicListData()
{
    return m_musicListView->getMusicListData();
}

void SearchResultTabWidget::setCurrentPage(ListPageSwitchType type)
{
    if (type == SearchMusicResultType) {
        if (m_musicListView) {
            m_StackedWidget->setCurrentWidget(m_musicListView);
        }
    } else if (type == SearchSingerResultType) {
        if (m_singerListView) {
            m_StackedWidget->setCurrentWidget(m_singerListView);
        }
    } else if (type == SearchAlbumResultType) {
        if (m_albumListView) {
            m_StackedWidget->setCurrentWidget(m_albumListView);
        }
    }
}

void SearchResultTabWidget::slotSizeChange()
{
    if (m_music->isEnabled()) {
        m_LineWidget->move((m_music->x() + (m_music->width() - m_LineWidget->width()) / 2), m_singer->y() + m_singer->height() + 8);
    } else if (m_singer->isEnabled()) {
        m_LineWidget->move((m_singer->x() + (m_singer->width() - m_LineWidget->width()) / 2), m_singer->y() + m_singer->height() + 8);
    } else if (m_album->isEnabled()) {
        m_LineWidget->move((m_album->x() + (m_album->width() - m_LineWidget->width()) / 2), m_singer->y() + m_singer->height() + 8);
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
        emit sigSearchTypeChanged("musicResult");
    } else if (m_singer->geometry().contains(event->pos())) {
        qDebug() << "zy------m_singer clicked";
        setLabelChecked(m_singer);
        emit sigSearchTypeChanged("artistResult");
    } else if (m_album->geometry().contains(event->pos())) {
        qDebug() << "zy------m_album clicked";
        setLabelChecked(m_album);
        emit sigSearchTypeChanged("albumResult");
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
        // 修改计算逻辑
        m_LineWidget->move((m_music->x() + (m_music->width() - m_LineWidget->width()) / 2), m_singer->y() + m_singer->height() + 8);
        m_StackedWidget->setCurrentWidget(m_musicListView);
    } else if (m_singer == label) {
        m_singer->setEnabled(true);
        m_album->setEnabled(false);
        m_music->setEnabled(false);
        m_LineWidget->show();
        m_LineWidget->move((m_singer->x() + (m_singer->width() - m_LineWidget->width()) / 2), m_singer->y() + m_singer->height() + 8);
        m_StackedWidget->setCurrentWidget(m_singerListView);
    } else if (m_album == label) {
        m_singer->setEnabled(false);
        m_album->setEnabled(true);
        m_music->setEnabled(false);
        m_LineWidget->show();
        m_LineWidget->move((m_album->x() + (m_album->width() - m_LineWidget->width()) / 2), m_singer->y() + m_singer->height() + 8);
        m_StackedWidget->setCurrentWidget(m_albumListView);
    }
}
