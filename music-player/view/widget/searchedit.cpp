/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "searchedit.h"

#include <QDebug>
#include <QKeyEvent>

#include <DUtil>
#include <thememanager.h>

#include "../../core/mediadatabase.h"
#include "../../core/music.h"
#include "searchresult.h"

DWIDGET_USE_NAMESPACE

SearchEdit::SearchEdit(QWidget *parent) : DSearchEdit(parent)
{
    ThemeManager::instance()->regisetrWidget(this, QStringList() << "viewname");

    connect(this, &SearchEdit::focusOut,
            this, &SearchEdit::onFocusOut);
//    connect(this, &SearchEdit::focusIn,
//            this, &SearchEdit::onFocusIn);
    connect(this, &SearchEdit::textChanged,
            this, &SearchEdit::onTextChanged);
    connect(this, &SearchEdit::returnPressed,
            this, &SearchEdit::onReturnPressed);
//    connect(this, &SearchEdit::editingFinished,
//            this, &SearchEdit::onReturnPressed);
}

void SearchEdit::setResultWidget(SearchResult *result)
{
    m_result = result;
    m_result->hide();

    connect(m_result, &SearchResult::locateMusic,
    this, [ = ](const QString & hash) {
//        onFocusOut();
        emit this->locateMusic(hash);
    });

    connect(m_result, &SearchResult::searchText,
    this, [ = ](const QString & text) {
//        onFocusOut();
        emit this->searchText(text);
    });
}

QString SearchEdit::viewname()
{
    return m_view;
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
    if (text.length() >= 2) {
        auto resultList = MediaDatabase::searchMediaTitle(text, 10);
        QStringList titleList;
        QStringList hashList;

        for (auto &meta : resultList) {
            titleList << meta->title;
            hashList << meta->hash;
        }

        auto searchtext = QString(this->text()).remove("\r").remove("\n");
        m_result->setFixedWidth(this->width() + 40);
        m_result->setSearchString(searchtext);
        m_result->setResultList(titleList, hashList);

        m_result->autoResize();
        m_result->show();
        // parent is MainFrame
        m_result->move(parentWidget()->rect().width() / 2  - 80 , 40 - 4);
        m_result->setFocusPolicy(Qt::StrongFocus);
        m_result->raise();
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

void SearchEdit::setViewname(QString viewname)
{
    if (m_view == viewname) {
        return;
    }

    m_view = viewname;
    emit viewnameChanged(viewname);

}
