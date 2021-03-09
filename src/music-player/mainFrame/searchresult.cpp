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

#include "searchresult.h"

#include <QDebug>
#include <QApplication>
#include <QStringListModel>

#include <DPushButton>
#include <DListView>
#include <DApplicationHelper>

#include "searchedit.h"
#include "ac-desktop-define.h"
#include "musicsearchlistview.h"

SearchResult::SearchResult(QWidget *parent) : DBlurEffectWidget(parent)
{
    QPalette labelPalette;
    labelPalette.setColor(QPalette::WindowText, QColor("#414D68 "));
    //设置圆角
    setBlurRectXRadius(18);
    setBlurRectYRadius(18);
    setRadius(30);
    setBlurEnabled(true);
    setMode(DBlurEffectWidget::GaussianBlur);

    vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(0, 8, 0, 18);
    vlayout->setSpacing(0);
    this->setLayout(vlayout);

    // 音乐
    m_MusicLabel = new DLabel(tr("Music"), this);
    m_MusicLabel->setFixedHeight(34);
    m_MusicLabel->setPalette(labelPalette);
    m_MusicLabel->setContentsMargins(32, 0, 0, 0);
    DFontSizeManager::instance()->bind(m_MusicLabel, DFontSizeManager::T8, QFont::Normal);

    m_MusicView = new MusicSearchListview(this);
    m_MusicView->setSearchResultWidget(this);
    m_MusicView->setSearchType(SearchType::SearchMusic);
    m_MusicView->setObjectName("SearchMusicView");
    m_MusicView->setMinimumWidth(380);

    AC_SET_OBJECT_NAME(m_MusicView, AC_musicView);
    AC_SET_ACCESSIBLE_NAME(m_MusicView, AC_musicView);

    // 分割线1
    s_ArtistLine = new DHorizontalLine;
    s_ArtistLine->setFrameShadow(DHorizontalLine::Raised);
    s_ArtistLine->setLineWidth(2);

    // 演唱者
    m_ArtistLabel = new DLabel(tr("Artists"), this);
    m_ArtistLabel->setFixedHeight(34);
    m_ArtistLabel->setPalette(labelPalette);
    m_ArtistLabel->setContentsMargins(32, 0, 0, 0);
    DFontSizeManager::instance()->bind(m_ArtistLabel, DFontSizeManager::T8, QFont::Normal);

    m_SingerView = new MusicSearchListview(this);
    m_SingerView->setObjectName("SearchArtistView");
    m_SingerView->setSearchType(SearchType::SearchSinger);
    m_SingerView->setMinimumWidth(380);
    m_SingerView->setSearchResultWidget(this);

    AC_SET_OBJECT_NAME(m_SingerView, AC_singerView);
    AC_SET_ACCESSIBLE_NAME(m_SingerView, AC_singerView);

    // 分割线2
    s_AblumLine = new DHorizontalLine;
    s_AblumLine->setFrameShadow(DHorizontalLine::Raised);
    s_AblumLine->setLineWidth(2);

    // 专辑
    m_AblumLabel = new DLabel(tr("Albums"), this);
    m_AblumLabel->setFixedHeight(34);
    m_AblumLabel->setPalette(labelPalette);
    m_AblumLabel->setContentsMargins(32, 0, 0, 0);
    DFontSizeManager::instance()->bind(m_AblumLabel, DFontSizeManager::T8, QFont::Normal);

    m_AlbumView = new MusicSearchListview(this);
    m_AlbumView->setObjectName("SearchAlbumView");
    m_AlbumView->setSearchType(SearchType::SearchAlbum);
    m_AlbumView->setMinimumWidth(380);
    m_AlbumView->setSearchResultWidget(this);

    AC_SET_OBJECT_NAME(m_AlbumView, AC_albumView);
    AC_SET_ACCESSIBLE_NAME(m_AlbumView, AC_albumView);

    vlayout->addWidget(m_MusicLabel, 1);
    vlayout->addWidget(m_MusicView, 0);
    vlayout->addWidget(s_ArtistLine);
    vlayout->addWidget(m_ArtistLabel, 1);
    vlayout->addWidget(m_SingerView, 0);
    vlayout->addWidget(s_AblumLine);
    vlayout->addWidget(m_AblumLabel, 1);
    vlayout->addWidget(m_AlbumView, 0);
    vlayout->addStretch(100);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &SearchResult::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());
//    connect(m_MusicView, &MusicSearchListview::clicked, this, &SearchResult::itemClicked);
//    connect(m_ArtistView, &MusicSearchListview::clicked, this, &SearchResult::itemClicked);
//    connect(m_AlbumView, &MusicSearchListview::clicked, this, &SearchResult::itemClicked);

//    connect(m_MusicView, &MusicSearchListview::sigSearchClear, this, &SearchResult::clearKeyState);
//    connect(m_ArtistView, &MusicSearchListview::sigSearchClear, this, &SearchResult::clearKeyState);
//    connect(m_AlbumView, &MusicSearchListview::sigSearchClear, this, &SearchResult::clearKeyState);
    autoResize();
}

SearchResult::~SearchResult()
{
}

void SearchResult::autoResize()
{
    this->show();
    int lineHeight = 0;
    int lineShowCount = 0;
    if (m_MusicView->rowCount() == 0) {
        m_MusicLabel->hide();
        m_MusicView->hide();
        s_ArtistLine->hide();
    } else {
        m_MusicLabel->show();
        m_MusicView->show();
        s_ArtistLine->show();
        s_AblumLine->show();
        lineShowCount++;
        m_MusicView->setFixedHeight(m_MusicView->rowCount() * 34);
    }

    if (m_SingerView->rowCount() == 0) {
        m_ArtistLabel->hide();
        m_SingerView->hide();
        s_ArtistLine->hide();
    } else {
        m_ArtistLabel->show();
        m_SingerView->show();
        s_AblumLine->show();
        lineShowCount++;
        m_SingerView->setFixedHeight(m_SingerView->rowCount() * 34);
    }

    if (m_AlbumView->rowCount() == 0) {
        m_AblumLabel->hide();
        m_AlbumView->hide();
        s_AblumLine->hide();
    } else {
        s_AblumLine->show();
        m_AblumLabel->show();
        m_AlbumView->show();
        lineShowCount++;
        m_AlbumView->setFixedHeight(m_AlbumView->rowCount() * 34);
    }

    if (lineShowCount > 0) {
        lineHeight = 2 * (lineShowCount - 1);
    }
    int total = 0;
    total += m_MusicLabel->isVisible() ? m_MusicLabel->height() : 0;
    total += m_MusicView->isVisible() ? m_MusicView->height() : 0;
//    total += s_ArtistLine->isVisible() ? s_ArtistLine->height() : 0;
    total += m_ArtistLabel->isVisible() ? m_ArtistLabel->height() : 0;
    total += m_SingerView->isVisible() ? m_SingerView->height() : 0;
//    total += s_AblumLine->isVisible() ? s_AblumLine->height() : 0;
    total += m_AblumLabel->isVisible() ? m_AblumLabel->height() : 0;
    total += m_AlbumView->isVisible() ? m_AlbumView->height() : 0;
    total += lineHeight;
    if (m_MusicView->isHidden() && m_SingerView->isHidden() && m_AlbumView->isHidden()) {
        setFixedHeight(total);
    } else {
        setFixedHeight(total + 28);
    }
}

void SearchResult::setSearchEdit(SearchEdit *edit)
{
    m_searchEdit = edit;
}

void SearchResult::setListviewSearchString(const QString &str)
{
    //搜索歌曲候选:<=5个
    m_MusicView->setSearchText(str);
    m_SingerView->setSearchText(str);
    m_AlbumView->setSearchText(str);
    autoResize();
    m_Count = m_MusicView->rowCount()
              + m_AlbumView->rowCount()
              + m_SingerView->rowCount();
    m_CurrentIndex = -1;
}

void SearchResult::setLineEditSearchString(const QString &str)
{
    if (m_searchEdit) {
        m_searchEdit->blockSignals(true);
        m_searchEdit->setText(str);
        m_searchEdit->blockSignals(false);
    }
}

void SearchResult::selectUp()
{
    if (m_Count == 0) {
        return;
    }
    if (m_CurrentIndex < 0) {
        return;
    }
    m_CurrentIndex --;
    if (m_CurrentIndex < m_MusicView->rowCount()) {
        m_MusicView->setCurrentIndexInt(m_CurrentIndex);
        m_SingerView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);
    } else if (m_CurrentIndex >= m_MusicView->rowCount() - 1
               && m_CurrentIndex < (m_MusicView->rowCount() + m_SingerView->rowCount())) {
        m_SingerView->setCurrentIndexInt(m_CurrentIndex
                                         - m_MusicView->rowCount());
        m_MusicView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);
    } else {
        m_AlbumView->setCurrentIndexInt(m_CurrentIndex
                                        - m_MusicView->rowCount()
                                        - m_SingerView->rowCount());
        m_MusicView->setCurrentIndexInt(-1);
        m_SingerView->setCurrentIndexInt(-1);
    }

    m_MusicView->update();
    m_SingerView->update();
    m_AlbumView->update();

    getSearchStr();
}

void SearchResult::selectDown()
{
    if (m_Count == 0) {
        return;
    }
    if (m_CurrentIndex >= m_Count - 1) {
        return;
    }

    m_CurrentIndex ++;

    if (m_CurrentIndex < m_MusicView->rowCount()) {
        m_MusicView->setCurrentIndexInt(m_CurrentIndex);

        m_SingerView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);

    } else if (m_CurrentIndex >= m_MusicView->rowCount() - 1
               && m_CurrentIndex < (m_MusicView->rowCount() + m_SingerView->rowCount())) {

        m_SingerView->setCurrentIndexInt(m_CurrentIndex
                                         - m_MusicView->rowCount());

        m_MusicView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);

    } else {
        m_AlbumView->setCurrentIndexInt(m_CurrentIndex
                                        - m_MusicView->rowCount()
                                        - m_SingerView->rowCount());

        m_MusicView->setCurrentIndexInt(-1);
        m_SingerView->setCurrentIndexInt(-1);

        m_MusicView->clearSelection();
        m_SingerView->clearSelection();
    }

    m_MusicView->update();
    m_SingerView->update();
    m_AlbumView->update();

    getSearchStr();
}

int SearchResult::getCurrentIndex()
{
    return m_CurrentIndex;
}

ListPageSwitchType SearchResult::getListPageSwitchType()
{
    if (m_MusicView && m_MusicView->rowCount() > 0) {
        return SearchMusicResultType;
    } else if (m_SingerView && m_SingerView->rowCount() > 0) {
        return SearchSingerResultType;
    } else {
        return SearchAlbumResultType;
    }
}

#if 0
void SearchResult::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setBrush(QColor("#f8f8f8"));
    painter.setPen(QPen(QColor("#E3E3E3"), 1));

    QRect rect = this->rect();
    rect.setWidth(rect.width());
    rect.setHeight(rect.height());
    painter.drawRoundedRect(rect, 18, 18);

    QWidget::paintEvent(event);
}
#endif

void SearchResult::leaveEvent(QEvent *event)
{
//    m_MusicView->setCurrentIndexInt(-1);
    DBlurEffectWidget::leaveEvent(event);
}

void SearchResult::onReturnPressed()
{
    qDebug() << "------SearchResult::onReturnPressed";
    if (m_MusicView->getIndexInt() >= 0) {
        m_MusicView->onReturnPressed();
    } else if (m_AlbumView->getIndexInt() >= 0) {
        m_AlbumView->onReturnPressed();
    } else if (m_SingerView->getIndexInt() >= 0) {
        m_SingerView->onReturnPressed();
    }
}

void SearchResult::setThemeType(int type)
{
    QPalette labelPalette;
    if (type == 2) {
        labelPalette.setColor(QPalette::Text, QColor("#FFF0F5 "));
    } else {
        labelPalette.setColor(QPalette::Text, QColor("#414D68 "));
    }
    m_MusicLabel->setPalette(labelPalette);
    m_ArtistLabel->setPalette(labelPalette);
    m_AblumLabel->setPalette(labelPalette);
}

//void SearchResult::itemClicked(QModelIndex index)
//{
//    PlaylistPtr playList = dynamic_cast<MusicSearchListview *>(index.model()->parent())->playlist();
//    QString currentId = playList->id();
//    int row = index.row();
//    qDebug() << "鼠标选中行:" << row;
//    if (currentId == MusicCandListID) {
//        Q_EMIT this->searchText2(MusicResultListID, playList->allmusic().at(row)->title);
//    }
//    if (currentId == AlbumCandListID) {
//        Q_EMIT this->searchText2(AlbumResultListID, playList->playMusicTypePtrList().at(row)->name);
//    }
//    if (currentId == ArtistCandListID) {
//        Q_EMIT this->searchText2(ArtistResultListID, playList->playMusicTypePtrList().at(row)->name);
//    }

//}

//键盘上下选择结果
void SearchResult::getSearchStr()
{
//    if (m_CurrentIndex < 0) {
//        return;
//    }
//    QString text;
//    QString id;
//    if (m_CurrentIndex < m_MusicView->rowCount()) {
//        text = m_MusicView->playlist()->allmusic().at(m_CurrentIndex)->title;
//        qDebug() << "当前选中歌曲:" << text;
//        id = MusicResultListID;
//    } else if (m_CurrentIndex >= m_MusicView->rowCount() - 1
//               && m_CurrentIndex < (m_MusicView->rowCount() + m_ArtistView->rowCount())) {
//        text = m_ArtistView->playlist()->playMusicTypePtrList().at(m_CurrentIndex - m_MusicView->rowCount())->name;
//        qDebug() << "当前选中歌手:" << text;
//        id = ArtistResultListID;
//    } else if (m_CurrentIndex >= m_MusicView->rowCount() + m_ArtistView->rowCount() - 1
//               && m_CurrentIndex < (m_MusicView->rowCount() + m_ArtistView->rowCount() + m_AlbumView->rowCount())) {
//        text = m_AlbumView->playlist()->playMusicTypePtrList().at(m_CurrentIndex - m_MusicView->rowCount() - m_ArtistView->rowCount())->name;
//        qDebug() << "当前选中专辑:" << text;
//        id = AlbumResultListID;
//    } else {
//        Q_EMIT this->searchText3("", "");
//        return;
//    }

//    Q_EMIT  this->searchText3(id, text);
}

//void SearchResult::clearKeyState()
//{
////    m_MusicView->setCurrentIndexInt(-1);
////    m_AlbumView->setCurrentIndexInt(-1);
////    m_ArtistView->setCurrentIndexInt(-1);

////    m_MusicView->update();
////    m_ArtistView->update();
////    m_AlbumView->update();
//}

