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

#include "searchresult.h"
#include "musicsearchlistview.h"

#include <QDebug>
#include <DPushButton>
#include <QStringListModel>

#include <DListView>
#include <DApplicationHelper>

#include "pushbutton.h"


SearchResult::SearchResult(QWidget *parent) : DBlurEffectWidget(parent)
{
    QFont labelFont("SourceHanSansSC");
    labelFont.setPointSize(DFontSizeManager::T9);
    QPalette labelPalette;
    labelPalette.setColor(QPalette::WindowText, QColor("#414D68 "));

    //背景 透明度
    auto effect = QColor("#f9f9f9");
    effect.setAlpha(240);
    setMaskColor(effect);

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

    //音乐
    m_MusicLabel = new DLabel(tr("Music"), this);
    m_MusicLabel->setFixedHeight(34);
    m_MusicLabel->setFont(labelFont);
    m_MusicLabel->setPalette(labelPalette);
    m_MusicLabel->setContentsMargins(32, 0, 0, 0);

    m_MusicView = new MusicSearchListview(this);
    m_MusicView->setObjectName("SearchMusicView");
    m_MusicView->setMinimumWidth(380);

    //分割线1
    s_ArtistLine = new DHorizontalLine;
    s_ArtistLine->setFrameShadow(DHorizontalLine::Raised);
    s_ArtistLine->setLineWidth(2);

    //演唱者
    m_ArtistLabel = new DLabel(tr("Artists"), this);
    m_ArtistLabel->setFixedHeight(34);
    m_ArtistLabel->setFont(labelFont);
    m_ArtistLabel->setPalette(labelPalette);
    m_ArtistLabel->setContentsMargins(32, 0, 0, 0);

    m_ArtistView = new MusicSearchListview(this);
    m_ArtistView->setObjectName("SearchArtistView");
    m_ArtistView->setMinimumWidth(380);

    //分割线2
    s_AblumLine = new DHorizontalLine;
    s_AblumLine->setFrameShadow(DHorizontalLine::Raised);
    s_AblumLine->setLineWidth(2);

    //专辑
    m_AblumLabel = new DLabel(tr("Albums"), this);
    m_AblumLabel->setFixedHeight(34);
    m_AblumLabel->setFont(labelFont);
    m_AblumLabel->setPalette(labelPalette);
    m_AblumLabel->setContentsMargins(32, 0, 0, 0);

    m_AlbumView = new MusicSearchListview(this);
    m_AlbumView->setObjectName("SearchAlbumView");
    m_AlbumView->setMinimumWidth(380);

    vlayout->addWidget(m_MusicLabel, 1);
    vlayout->addWidget(m_MusicView, 0);
    vlayout->addWidget(s_ArtistLine);
    vlayout->addWidget(m_ArtistLabel, 1);
    vlayout->addWidget(m_ArtistView, 0);
    vlayout->addWidget(s_AblumLine);
    vlayout->addWidget(m_AblumLabel, 1);
    vlayout->addWidget(m_AlbumView, 0);
    vlayout->addStretch(100);

    int themeType = DGuiApplicationHelper::instance()->themeType();
    slotTheme(themeType);
    connect(m_MusicView, &MusicSearchListview::clicked, this, &SearchResult::itemClicked);
    connect(m_ArtistView, &MusicSearchListview::clicked, this, &SearchResult::itemClicked);
    connect(m_AlbumView, &MusicSearchListview::clicked, this, &SearchResult::itemClicked);

    connect(m_MusicView, &MusicSearchListview::sigSearchClear, this, &SearchResult::clearKeyState);
    connect(m_ArtistView, &MusicSearchListview::sigSearchClear, this, &SearchResult::clearKeyState);
    connect(m_AlbumView, &MusicSearchListview::sigSearchClear, this, &SearchResult::clearKeyState);
}

void SearchResult::autoResize()
{
    int rowCount = m_MusicView->rowCount();

    int musicHeight     = 0;
    int artistHeight    = 0;
    int ablumHeight     = 0;

    m_MusicView->setFixedHeight(rowCount * 34);
    m_MusicView->raise();
    musicHeight = (rowCount == 0 ? 0 : (rowCount + 1) * 34);

    rowCount = m_ArtistView->rowCount();
    m_ArtistView->setFixedHeight(rowCount * 34);
    m_ArtistView->raise();
    artistHeight = (rowCount == 0 ? 0 : (rowCount + 1) * 34);

    rowCount = m_AlbumView->rowCount();
    m_AlbumView->setFixedHeight(rowCount * 34);
    m_AlbumView->raise();
    ablumHeight = (rowCount == 0 ? 0 : (rowCount + 1) * 34);


    if (m_MusicView->rowCount() == 0) {
        m_MusicLabel->hide();
        m_MusicView->hide();
        s_ArtistLine->hide();
    } else {
        m_MusicLabel->show();
        m_MusicView->show();
        s_ArtistLine->show();
        s_AblumLine->show();
    }


    if (m_ArtistView->rowCount() == 0) {
        m_ArtistLabel->hide();
        m_ArtistView->hide();
        s_ArtistLine->hide();
    } else {
        m_ArtistLabel->show();
        m_ArtistView->show();
        s_AblumLine->show();
    }

    if (m_AlbumView->rowCount() == 0) {
        m_AblumLabel->hide();
        m_AlbumView->hide();
        s_AblumLine->hide();
    } else {
        m_AblumLabel->show();
        m_AlbumView->show();
    }

    if (m_MusicView->rowCount() == 0
            && m_ArtistView->rowCount() == 0
            && m_AlbumView->rowCount() == 0) {
        hide();
    }

    int total = 0;
    total += m_MusicLabel->isVisible() ? m_MusicLabel->height() : 0;
    total += m_MusicView->isVisible() ? m_MusicView->height() : 0;
    total += s_ArtistLine->isVisible() ? s_ArtistLine->height() : 0;
    total += m_ArtistLabel->isVisible() ? m_ArtistLabel->height() : 0;
    total += m_ArtistView->isVisible() ? m_ArtistView->height() : 0;
    total += s_AblumLine->isVisible() ? s_AblumLine->height() : 0;
    total += m_AblumLabel->isVisible() ? m_AblumLabel->height() : 0;
    total += m_AlbumView->isVisible() ? m_AlbumView->height() : 0;
    setFixedHeight(total + 26);
}

void SearchResult::setSearchString(const QString &str)
{
    m_MusicView->setProperty("searchString", str);
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
        m_ArtistView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);
    } else if (m_CurrentIndex >= m_MusicView->rowCount() - 1
               && m_CurrentIndex < (m_MusicView->rowCount() + m_ArtistView->rowCount())) {
        m_ArtistView->setCurrentIndexInt(m_CurrentIndex
                                         - m_MusicView->rowCount());
        m_MusicView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);
    } else {
        m_AlbumView->setCurrentIndexInt(m_CurrentIndex
                                        - m_MusicView->rowCount()
                                        - m_ArtistView->rowCount());
        m_MusicView->setCurrentIndexInt(-1);
        m_ArtistView->setCurrentIndexInt(-1);
    }

    m_MusicView->update();
    m_ArtistView->update();
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

        m_ArtistView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);

    } else if (m_CurrentIndex >= m_MusicView->rowCount() - 1
               && m_CurrentIndex < (m_MusicView->rowCount() + m_ArtistView->rowCount())) {

        m_ArtistView->setCurrentIndexInt(m_CurrentIndex
                                         - m_MusicView->rowCount());

        m_MusicView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);

    } else {
        m_AlbumView->setCurrentIndexInt(m_CurrentIndex
                                        - m_MusicView->rowCount()
                                        - m_ArtistView->rowCount());

        m_MusicView->setCurrentIndexInt(-1);
        m_ArtistView->setCurrentIndexInt(-1);

        m_MusicView->clearSelection();
        m_ArtistView->clearSelection();
    }

    m_MusicView->update();
    m_ArtistView->update();
    m_AlbumView->update();

    getSearchStr();
}

QString SearchResult::currentStr()
{
    QString str;

    return str;
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
    m_MusicView->setCurrentIndexInt(-1);
    DBlurEffectWidget::leaveEvent(event);
}

void SearchResult::onReturnPressed()
{
    //    auto hashlist = m_MusicModel->property("hashlist").toStringList();
    //    auto selectedRow = m_MusicView->currentIndex().row();
    //    Q_EMIT locateMusic(hashlist.value(selectedRow));
}

void SearchResult::selectPlaylist(PlaylistPtr playlistPtr)
{
    playlist = playlistPtr;
}

void SearchResult::onSearchCand(QString text, PlaylistPtr playlistPtr)
{
    m_CurrentIndex = -1;
    if (playlistPtr->id() == MusicCandListID) {
        m_MusicView->onMusiclistChanged(text, playlistPtr);
    }
    if (playlistPtr->id() == AlbumCandListID) {
        m_AlbumView->onMusiclistChanged(text, playlistPtr);
    }
    if (playlistPtr->id() == ArtistCandListID) {
        m_ArtistView->onMusiclistChanged(text, playlistPtr);
    }
    m_MusicView->setCurrentIndexInt(-1);
    m_AlbumView->setCurrentIndexInt(-1);
    m_ArtistView->setCurrentIndexInt(-1);
    m_Count = m_MusicView->rowCount()
              + m_AlbumView->rowCount()
              + m_ArtistView->rowCount();
    autoResize();
}

void SearchResult::slotTheme(int type)
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
    m_MusicView->setThemeType(type);
    m_AlbumView->setThemeType(type);
    m_ArtistView->setThemeType(type);
}

void SearchResult::itemClicked(QModelIndex index)
{
    PlaylistPtr playList = dynamic_cast<MusicSearchListview *>(index.model()->parent())->playlist();
    QString currentId = playList->id();
    int row = index.row();
    qDebug() << "鼠标选中行:" << row;
    if (currentId == MusicCandListID) {
        Q_EMIT this->searchText2(MusicResultListID, playList->allmusic().at(row)->title);
    }
    if (currentId == AlbumCandListID) {
        Q_EMIT this->searchText2(AlbumResultListID, playList->playMusicTypePtrList().at(row)->name);
    }
    if (currentId == ArtistCandListID) {
        Q_EMIT this->searchText2(ArtistResultListID, playList->playMusicTypePtrList().at(row)->name);
    }

}

//键盘上下选择结果
void SearchResult::getSearchStr()
{
    if (m_CurrentIndex < 0) {
        return;
    }
    QString text;
    QString id;
    if (m_CurrentIndex < m_MusicView->rowCount()) {
        text = m_MusicView->playlist()->allmusic().at(m_CurrentIndex)->title;
        qDebug() << "当前选中歌曲:" << text;
        id = MusicResultListID;
    } else if (m_CurrentIndex >= m_MusicView->rowCount() - 1
               && m_CurrentIndex < (m_MusicView->rowCount() + m_ArtistView->rowCount())) {
        text = m_ArtistView->playlist()->playMusicTypePtrList().at(m_CurrentIndex - m_MusicView->rowCount())->name;
        qDebug() << "当前选中歌手:" << text;
        id = ArtistResultListID;
    } else if (m_CurrentIndex >= m_MusicView->rowCount() + m_ArtistView->rowCount() - 1
               && m_CurrentIndex < (m_MusicView->rowCount() + m_ArtistView->rowCount() + m_AlbumView->rowCount())) {
        text = m_AlbumView->playlist()->playMusicTypePtrList().at(m_CurrentIndex - m_MusicView->rowCount() - m_ArtistView->rowCount())->name;
        qDebug() << "当前选中专辑:" << text;
        id = AlbumResultListID;
    } else {
        Q_EMIT this->searchText3("", "");
        return;
    }

    Q_EMIT  this->searchText3(id, text);
}

void SearchResult::clearKeyState()
{
    m_MusicView->setCurrentIndexInt(-1);
    m_AlbumView->setCurrentIndexInt(-1);
    m_ArtistView->setCurrentIndexInt(-1);

    m_MusicView->update();
    m_ArtistView->update();
    m_AlbumView->update();
}



