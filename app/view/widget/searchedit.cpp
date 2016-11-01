/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "searchedit.h"

#include <QKeyEvent>

#include "searchresult.h"

#include "../../core/mediadatabase.h"
#include "../../model/musiclistmodel.h"

SearchEdit::SearchEdit(QWidget *parent) : DSearchEdit(parent)
{
    m_result = new SearchResult();
    m_result->setFixedWidth(278);
    m_result->hide();

    connect(m_result, &SearchResult::locateMusic,
            this, &SearchEdit::locateMusic);
    connect(m_result, &SearchResult::searchText,
            this, &SearchEdit::searchText);

    connect(this, &SearchEdit::focusOut,
            this, &SearchEdit::onFocusOut);
//    connect(this, &SearchEdit::focusIn,
//            this, &SearchEdit::onFocusIn);
    connect(this, &SearchEdit::textChanged,
            this, &SearchEdit::onTextChanged);
    connect(this, &SearchEdit::returnPressed,
            this, &SearchEdit::onReturnPressed);
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
    m_result->hide();
    m_result->close();
}

void SearchEdit::onTextChanged()
{
    auto text = QString(this->text()).remove(" ");
    if (text.length() >= 2) {
        auto resultList = MediaDatabase::searchMusicTitle(text, 10);
        QStringList titleList;
        QStringList hashList;

        for (auto &meta : resultList) {
            titleList << meta.title;
            hashList << meta.hash;
        }
        m_result->setSearchString(this->text());
        m_result->setResultList(titleList, hashList);

        m_result->autoResize();
        auto pos = this->mapToGlobal(QPoint(0, this->height() + 2));
        m_result->show();
        m_result->move(pos);
    } else {
        onFocusOut();
    }
}

void SearchEdit::onReturnPressed()
{
    auto text = QString(this->text()).remove(" ");
    if (text.isEmpty()) {
        return;
    }

    onFocusOut();

    if (m_result->isSelected()) {
        m_result->onReturnPressed();
    } else {
        emit this->searchText(this->text());
    }
}
