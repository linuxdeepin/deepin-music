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
    auto textFont = font();
    textFont.setPixelSize(14);
    setFont(textFont);

    lineEdit()->setFocusPolicy(Qt::ClickFocus);
    // Why qss not work if not call show
    //    show();
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
//            if (this->text().isEmpty())
//                onReturnPressed();
            //            else
            //                clear();
        } else {
            onTextChanged();
        }
    });
}

void SearchEdit::setResultWidget(SearchResult *result)
{
    m_result = result;

    m_result->hide();
    m_result->move(this->x(), this->y() + 50);
    connect(m_result, &SearchResult::searchText,
    this, [ = ](const QString & id, const QString & text) {
        onFocusOut();
        setText(text);
        onFocusOut();
        Q_EMIT this->searchText(id, text);
    });

    connect(m_result, &SearchResult::searchText2,
    this, [ = ](const QString & id, const QString & text) {
        searchText2(id, text);
    });
}

void SearchEdit::keyPressEvent(QKeyEvent *event)
{
    //输入框中上下按键操作
    if (event->key() == Qt::Key_Up) {
        m_result->selectUp();
    }
    if (event->key() == Qt::Key_Down) {
        m_result->selectDown();
    }

    DSearchEdit::keyPressEvent(event);
}

void SearchEdit::searchText2(QString id, QString text)
{
    m_CurrentId = id;
    m_Text = text;
}

void SearchEdit::onFocusIn()
{
    setText("");
    m_CurrentId = "";
    m_result->adjustSize();
    auto pos = this->mapToGlobal(QPoint(0, this->height() + 2));
    m_result->show();
    m_result->move(pos);
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
    if (m_LastText == text) {
        return;
    }
    m_LastText = text;
    if (text.length() >= 1) {
        m_result->setSearchString(text);
        m_result->show();
        // parent is MainFrame
        QRect rect = this->rect();
        QPoint bottomLeft = rect.bottomLeft();
        bottomLeft = mapTo(parentWidget()->parentWidget(), bottomLeft);
        m_result->setFixedWidth(width() - 4);
        m_result->move(bottomLeft.x() + width() / 2 + 24, bottomLeft.y());
        m_result->setFocusPolicy(Qt::StrongFocus);
        m_result->raise();
    } else {
        onFocusOut();
    }
    if (text.size() != 0) {
        Q_EMIT this->searchCand(text);
    }
}

void SearchEdit::onReturnPressed()
{
    if (!m_result->currentStr().isEmpty())
        setText(m_result->currentStr());
    auto text = QString(this->text()).remove(" ").remove("\r").remove("\n");;
    if (text.length() == 0)
        return;
    onFocusOut();
    if (m_CurrentId.size() == 0) {
        Q_EMIT this->searchText("", text);
    } else {
        Q_EMIT this->searchText(m_CurrentId, m_Text);
    }
}

