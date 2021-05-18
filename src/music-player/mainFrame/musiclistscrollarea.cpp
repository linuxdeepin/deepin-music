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

#include "musiclistscrollarea.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QFocusEvent>
#include <QScrollBar>

#include <DListWidget>
#include <DLabel>
#include <DPushButton>
#include <DFloatingButton>
#include <DPalette>
#include <DUtil>
#include <DGuiApplicationHelper>
#include <DApplicationHelper>

#include "../core/musicsettings.h"
//#include "widget/musiclistview.h"
#include "listView/musicBaseAndSongList/musicbaselistview.h"
#include "listView/musicBaseAndSongList/musicsonglistview.h"
#include "commonservice.h"
#include "ac-desktop-define.h"

MusicListScrollArea::MusicListScrollArea(QWidget *parent) : DScrollArea(parent)
{
    setFrameShape(QFrame::NoFrame);
    setWidgetResizable(true);

    setAutoFillBackground(true);
    auto leftFramePalette = palette();
    leftFramePalette.setColor(DPalette::Background, QColor("#FFFFFF"));
    setPalette(leftFramePalette);

    QWidget *widget = new QWidget(this);
    setWidget(widget);
    if (CommonService::getInstance()->isTabletEnvironment()) {
        setFixedWidth(300);
        widget->setFixedWidth(300);
    } else {
        setFixedWidth(220);
        widget->setFixedWidth(220);
    }
    auto musicLayout = new QVBoxLayout(widget);
    musicLayout->setContentsMargins(10, 5, 10, 5);
    musicLayout->setSpacing(0);

    dataBaseLabel = new DLabel;
    dataBaseLabel->setFixedHeight(40);
    dataBaseLabel->setText(tr("Library"));
    dataBaseLabel->setObjectName("MusicListScrollAreaDataBase");
    dataBaseLabel->setMargin(10);
    auto dataBaseLabelFont = dataBaseLabel->font();
    dataBaseLabelFont.setFamily("SourceHanSansSC");
    dataBaseLabelFont.setWeight(QFont::Medium);

    // 解决字体不会根据系统字体大小改变问题
    dataBaseLabel->setFont(dataBaseLabelFont);
    auto dataBaseLabelLayout = new QHBoxLayout();
    dataBaseLabelLayout->setContentsMargins(0, 0, 15, 0);
    dataBaseLabelLayout->addWidget(dataBaseLabel, 100, Qt::AlignLeft | Qt::AlignVCenter);
    dataBaseLabelLayout->addStretch();

    customizeLabel = new DLabel;
    customizeLabel->setFixedHeight(40);
    customizeLabel->setText(tr("Playlists"));
    customizeLabel->setObjectName("MusicListScrollAreaCustomizeLabel");
    customizeLabel->setMargin(10);
    customizeLabel->setFont(dataBaseLabelFont);

    m_addListBtn = new DIconButton(this);
    m_addListBtn->setIcon(QIcon::fromTheme("text_add"));
    m_addListBtn->setEnabledCircle(true);
    m_addListBtn->setIconSize(QSize(20, 20));
    m_addListBtn->setFixedSize(26, 26);

    m_addListBtn->setFocusPolicy(Qt::TabFocus);
    m_addListBtn->installEventFilter(this);

    auto customizeLayout = new QHBoxLayout();
    customizeLayout->setContentsMargins(0, 0, 15, 0);
    customizeLayout->addWidget(customizeLabel, 100, Qt::AlignLeft);
    customizeLayout->addStretch();
    customizeLayout->addWidget(m_addListBtn, 0, Qt::AlignRight);

    m_dataBaseListview = new MusicBaseListView;
    m_dataBaseListview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_dataBaseListview->setFixedHeight(162);
    AC_SET_OBJECT_NAME(m_dataBaseListview, AC_dataBaseListview);
    AC_SET_ACCESSIBLE_NAME(m_dataBaseListview, AC_dataBaseListview);

    // m_dataBaseListview->setFocusPolicy(Qt::TabFocus);   //使用默认设置焦点方式
    m_dataBaseListview->installEventFilter(this);

    m_customizeListview = new MusicSongListView;
    musicLayout->setContentsMargins(0, 0, 0, 0);

    // m_customizeListview->setFocusPolicy(Qt::TabFocus);
    m_customizeListview->installEventFilter(this);

    musicLayout->addLayout(dataBaseLabelLayout, 0);
    musicLayout->addWidget(m_dataBaseListview, 0, Qt::AlignTop);
    musicLayout->addLayout(customizeLayout);
    musicLayout->addWidget(m_customizeListview);
    AC_SET_OBJECT_NAME(m_customizeListview, AC_customizeListview);
    AC_SET_ACCESSIBLE_NAME(m_customizeListview, AC_customizeListview);

    slotTheme(DGuiApplicationHelper::instance()->themeType());

    connect(m_addListBtn, SIGNAL(clicked()), m_customizeListview, SLOT(addNewSongList()));

    //达到添加新歌单后滚动条滑到当前新增项
    connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, [ = ](int min, int max) {
        Q_UNUSED(min)
        if (max > 0 && m_customizeListview->getHeightChangeToMax()) {
            verticalScrollBar()->setValue(max);
        }
    });

    connect(m_dataBaseListview, &QAbstractItemView::clicked, this, &MusicListScrollArea::slotListViewClicked);
    connect(m_customizeListview, &QAbstractItemView::clicked, this, &MusicListScrollArea::slotListViewClicked);
    connect(m_customizeListview, &MusicSongListView::sigAddNewSongList, this, &MusicListScrollArea::slotAddNewSongList);
    connect(CommonService::getInstance(), &CommonService::signalSwitchToView, this, &MusicListScrollArea::viewChanged);
}

void MusicListScrollArea::slotTheme(int type)
{
    if (type == 0)
        type = DGuiApplicationHelper::instance()->themeType();
    if (type == 1) {
        DPalette leftFramePalette = palette();
        leftFramePalette.setColor(DPalette::Background, QColor("#FFFFFF"));
        setPalette(leftFramePalette);

        DPalette dataBaseLabelPalette = dataBaseLabel->palette();
        QColor WindowTextColor("#757F92");
        dataBaseLabelPalette.setColor(DPalette::WindowText, WindowTextColor);
        DApplicationHelper::instance()->setPalette(dataBaseLabel, dataBaseLabelPalette);
        DApplicationHelper::instance()->setPalette(customizeLabel, dataBaseLabelPalette);
    } else {
        auto leftFramePalette = palette();
        leftFramePalette.setColor(DPalette::Background, QColor("#232323"));
        setPalette(leftFramePalette);

        DPalette dataBaseLabelPalette = dataBaseLabel->palette();
        // 根据设计重新设置颜色
        QColor WindowTextColor("#EDEDED");
        dataBaseLabelPalette.setColor(DPalette::WindowText, WindowTextColor);
        DApplicationHelper::instance()->setPalette(dataBaseLabel, dataBaseLabelPalette);
        DApplicationHelper::instance()->setPalette(customizeLabel, dataBaseLabelPalette);
    }
}

void MusicListScrollArea::slotListViewClicked(const QModelIndex &index)
{
    Q_UNUSED(index)
    DListView *listview =   dynamic_cast<DListView *>(sender());
    if (listview == m_dataBaseListview) {
        m_customizeListview->clearSelection();
    } else if (listview == m_customizeListview) {
        m_dataBaseListview->clearSelection();
    }
}

void MusicListScrollArea::slotAddNewSongList()
{
    m_dataBaseListview->clearSelection();
}

void MusicListScrollArea::viewChanged(ListPageSwitchType switchtype, const QString &hashOrSearchword)
{
    Q_UNUSED(hashOrSearchword)
    switch (switchtype) {
    case AlbumType:
    case AllSongListType:
    case SingerType:
    case FavType:
    case CdaType:
    case CustomType: {
        emit CommonService::getInstance()->signalClearEdit();//清空搜索栏
        return;
    }
    case SearchMusicResultType:
    case SearchSingerResultType:
    case SearchAlbumResultType: {
        //搜索歌曲结果页面，清空选中
        m_customizeListview->clearSelection();
        m_dataBaseListview->clearSelection();
        return;
    }
    default:
        return;
    }
}

bool MusicListScrollArea::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_dataBaseListview) {

        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *event = static_cast<QKeyEvent *>(e);
            if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_M)) {

                int rowIndex = m_dataBaseListview->currentIndex().row();
                int row = 40 * rowIndex;

                QPoint pos(120, row);
                m_dataBaseListview->showContextMenu(pos);
            }
        }
        // Tab焦点进入事件和点击事件冲突，保留点击事件设置焦点
    }

    if (o == m_customizeListview) {

        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *event = static_cast<QKeyEvent *>(e);
            if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_M)) {

                int rowIndex = m_customizeListview->currentIndex().row();
                int row = 40 * rowIndex;
                QPoint pos;

                if (row > 300) {
                    QPoint posm(120, 120);
                    pos = posm;
                } else {
                    QPoint posm(120, row);
                    pos = posm;
                }

                m_customizeListview->showContextMenu(pos);
            }
        }
        // Tab焦点进入事件和点击事件冲突，保留点击事件设置焦点
    }

    if (o == m_addListBtn) {

        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *event = static_cast<QKeyEvent *>(e);
            if (event->key() == Qt::Key_Return) {

                Q_EMIT m_addListBtn->click();
            }
        } else if (e->type() == QEvent::FocusIn) {
            m_dataBaseListview->clearSelection();
        }
    }

    return QWidget::eventFilter(o, e);
}

void MusicListScrollArea::resizeEvent(QResizeEvent *event)
{
    m_customizeListview->adjustHeight();
    DScrollArea::resizeEvent(event);
    sizeHint();
}

