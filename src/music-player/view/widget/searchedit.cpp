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

#include "searchedit.h"

#include <QDebug>
#include <QKeyEvent>

#include <DUtil>

#include "util/pinyinsearch.h"

#include "../../core/mediadatabase.h"
#include "../../core/music.h"
#include "searchresult.h"

DWIDGET_USE_NAMESPACE

SearchEdit::SearchEdit(QWidget *parent) : DSearchEdit(parent)
{
    // Why qss not work if not call show
    show();
//    connect(this, &SearchEdit::focusOut,
//            this, &SearchEdit::onFocusOut);
//    connect(this, &SearchEdit::focusIn,
//            this, &SearchEdit::onFocusIn);
    connect(this, &SearchEdit::textChanged,
            this, &SearchEdit::onTextChanged);
    connect(this, &SearchEdit::returnPressed,
            this, &SearchEdit::onReturnPressed);
//    connect(this, &SearchEdit::editingFinished,
//            this, &SearchEdit::onReturnPressed);
    connect(this, &SearchEdit::focusChanged,
    this, [ = ](bool onFocus) {
        if (!onFocus) {
            m_result->hide();
            onReturnPressed();
        } else {
            onTextChanged();
        }
    });
}

void SearchEdit::setResultWidget(SearchResult *result)
{
    m_result = result;

    m_result->hide();

    connect(m_result, &SearchResult::locateMusic,
    this, [ = ](const QString & hash) {
//        onFocusOut();
        Q_EMIT this->locateMusic(hash);
    });

    connect(m_result, &SearchResult::searchText,
    this, [ = ](const QString & text) {
//        onFocusOut();
        setText(text);
        onReturnPressed();
    });
}

void SearchEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        m_result->selectUp();
    }
    if (event->key() == Qt::Key_Down) {
        m_result->selectDown();
    }

    // TODO: event fiter here tab cap by line edit
//    if (event->key() == Qt::Key_Backtab) {
//        m_result->selectUp();
//        event->accept();
//        return;
//    }
//    if (event->key() == Qt::Key_Tab) {
//        m_result->selectDown();
//        event->accept();
//        return;
//    }
    DSearchEdit::keyPressEvent(event);
}

void SearchEdit::onFocusIn()
{
//    m_result->adjustSize();
//    auto pos = this->mapToGlobal(QPoint(0, this->height() + 2));
//    m_result->show();
//    m_result->move(pos);
}

void SearchEdit::onFocusOut()
{
    DUtil::TimerSingleShot(50, [ this ]() {
        m_result->hide();
        m_result->close();
    });
}

void SearchEdit::onTextChanged()
{
    auto text = QString(this->text()).remove(" ").remove("\r").remove("\n");
    if (text.length() >= 1) {
        auto searchtext = QString(this->text()).remove("\r").remove("\n");
        QRect rect = this->rect();

        QStringList curList;
        if (playlist->id() == AlbumMusicListID || playlist->id() == ArtistMusicListID) {
            PlayMusicTypePtrList playMusicTypePtrList = playlist->playMusicTypePtrList();

            for (auto action : playMusicTypePtrList) {
                if (!curList.contains(action->name))
                    curList.append(action->name);
            }
        } else {
            for (auto action : playlist->allmusic()) {
                if (!curList.contains(action->title))
                    curList.append(action->title);
            }
        }
        //filter
        bool chineseFlag = false;
        for (auto ch : text) {
            if (DMusic::PinyinSearch::isChinese(ch)) {
                chineseFlag = true;
                break;
            }
        }
        QStringList filterList;
        if (chineseFlag) {
            filterList = curList.filter(searchtext, Qt::CaseInsensitive);
        } else {
            if (text.size() == 1) {
                for (auto curText : curList) {
                    auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(curText);
                    if (!curTextList.isEmpty() && curTextList.first().contains(searchtext, Qt::CaseInsensitive)) {
                        filterList.append(curText);
                    }
                }
            } else {
                for (auto curText : curList) {
                    auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(curText);
                    if (!curTextList.isEmpty() && curTextList.join("").contains(searchtext, Qt::CaseInsensitive)) {
                        filterList.append(curText);
                    }
                }
            }
        }

        if (filterList.size() > 10)
            filterList = filterList.mid(0, 10);

        m_result->setSearchString(text);
        m_result->setResultList(filterList, QStringList());

        m_result->autoResize();
        m_result->show();
        // parent is MainFrame
        QPoint bottomLeft = rect.bottomLeft();
        bottomLeft = mapTo(parentWidget()->parentWidget(), bottomLeft);
        m_result->move(bottomLeft.x() + width() / 2 + 24, bottomLeft.y() + 5);
        m_result->setFocusPolicy(Qt::StrongFocus);
        m_result->raise();
    } else {
        onFocusOut();
    }
}

void SearchEdit::onReturnPressed()
{
    if (!m_result->currentStr().isEmpty())
        setText(m_result->currentStr());
    auto text = QString(this->text()).remove(" ");
//    if (text.isEmpty()) {
//        return;
//    }

    onFocusOut();

    if (playlist != nullptr)
        playlist->setSearchStr(text);
    Q_EMIT this->searchText(this->text());
}

void SearchEdit::selectPlaylist(PlaylistPtr playlistPtr)
{
    playlist = playlistPtr;
}
