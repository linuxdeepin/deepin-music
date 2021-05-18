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

#include "searchedit.h"

#include <QDebug>
#include <QKeyEvent>

#include <DUtil>
#include <DApplication>

#include "util/pinyinsearch.h"

#include "../../core/music.h"
#include "searchresult.h"

#include "ac-desktop-define.h"
#include "commonservice.h"

DWIDGET_USE_NAMESPACE

SearchEdit::SearchEdit(QWidget *parent) : DSearchEdit(parent)
{
    auto textFont = font();

// 解决字体不会根据系统字体大小改变问题
//    textFont.setPixelSize(14);
    setFont(textFont);

    lineEdit()->setFocusPolicy(Qt::ClickFocus);
    lineEdit()->setContextMenuPolicy(Qt::DefaultContextMenu);

    AC_SET_OBJECT_NAME(lineEdit(), AC_Search);
    AC_SET_ACCESSIBLE_NAME(lineEdit(), AC_Search);

    connect(this, &SearchEdit::textChanged,
            this, &SearchEdit::onTextChanged);
    connect(this, &SearchEdit::returnPressed,
            this, &SearchEdit::onReturnPressed);
    connect(this, &SearchEdit::focusChanged,
    this, [ = ](bool onFocus) {
        qDebug() << "zy------SearchEdit::focusChanged onFocus = " << onFocus;
        if (m_result == nullptr) {
            m_result = new SearchResult(m_mainWindow);
            m_result->setSearchEdit(this);
            QRect rect = this->rect();
            QPoint bottomLeft = rect.bottomLeft();
            bottomLeft = mapTo(parentWidget()->parentWidget(), bottomLeft);
            m_result->setFixedWidth(width());
            m_result->hide();
            m_result->move(bottomLeft.x(), bottomLeft.y() + 5);
            if (CommonService::getInstance()->isTabletEnvironment()) {
                if (!DApplication::inputMethod()->isVisible()) {
                    DApplication::inputMethod()->show();
                }
            }
        }
        if (!onFocus) {
            m_result->hide();
            if (lineEdit()) {
                lineEdit()->clearFocus();
            }
            this->clearFocus();
        }
    });
}

//void SearchEdit::setResultWidget(SearchResult *result)
//{
//    m_result = result;
//    m_result->setSearchEdit(this);

//    QRect rect = this->rect();
//    QPoint bottomLeft = rect.bottomLeft();
//    bottomLeft = mapTo(parentWidget()->parentWidget(), bottomLeft);
//    m_result->setFixedWidth(width());
//    m_result->hide();
//    m_result->move(bottomLeft.x(), bottomLeft.y() + 5);
//}

void SearchEdit::setMainWindow(QWidget *mainWindow)
{
    m_mainWindow = mainWindow;
}

void SearchEdit::keyPressEvent(QKeyEvent *event)
{
    //输入框中上下按键操作
    if (m_result == nullptr) {
        return;
    }
    if (event->key() == Qt::Key_Up) {
        m_result->selectUp();
    }
    if (event->key() == Qt::Key_Down) {
        m_result->selectDown();
    }
    DSearchEdit::keyPressEvent(event);
}

//void SearchEdit::onFocusIn()
//{
//    setText("");
//    m_CurrentId = "";
//    if (m_result == nullptr) {
//        m_result = new SearchResult();
//        m_result->setFixedWidth(this->width());
//    }
////    m_result->adjustSize();
////    auto pos = this->mapToGlobal(QPoint(0, this->height() + 2));
////    m_result->show();
////    m_result->raise();
////    m_result->move(pos);
//}

void SearchEdit::onTextChanged()
{
    if (m_result == nullptr) {
        return;
    }
    auto alltext = this->text();
    if (alltext.isEmpty()) {
        m_result->hide();
        return;
    }

    //设置光标
    lineEdit()->setCursorPosition(lineEdit()->cursorPosition());

    auto text = QString(this->text()).remove("\r").remove("\n");

    if (this->text().size() == 0) {
        m_result->hide();
//        m_LastText = "";
        return;
    }
//    if (m_Text == this->text()) {
//        return;
//    }
    m_CurrentId = "";
//    if (m_LastText == text) {
//        return;
//    }
//    m_LastText = text;
    if (text.length() >= 1) {
        m_result->setListviewSearchString(text);
        m_result->show();
//        auto pos = this->mapToGlobal(QPoint(0, this->height() + 2));
//        m_result->move(pos);

        // parent is MainFrame
        QRect rect = this->rect();
        QPoint bottomLeft = rect.bottomLeft();
        bottomLeft = mapTo(parentWidget()->parentWidget(), bottomLeft);
        m_result->setFixedWidth(width());
        m_result->move(bottomLeft.x(), bottomLeft.y() + 5);
        m_result->setFocusPolicy(Qt::StrongFocus);
        m_result->raise();
    } else {
        m_result->hide();
    }
}

void SearchEdit::onReturnPressed()
{
    if (m_result == nullptr) {
        return;
    }
    if (m_result->getCurrentIndex() >= 0) {
        m_result->onReturnPressed();
        m_result->hide();
        return;
    }
    auto text = QString(this->text()).remove("\r").remove("\n");
    if (text.length() == 0)
        return;
    m_result->hide();
    emit CommonService::getInstance()->signalSwitchToView(m_result->getListPageSwitchType(), text);
}


