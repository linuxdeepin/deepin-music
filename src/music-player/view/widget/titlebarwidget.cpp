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

#include "titlebarwidget.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QApplication>
#include <QFocusEvent>
#include <QGraphicsOpacityEffect>

#include <DWidgetUtil>
#include <DPushButton>
#include <DSearchEdit>
#include <DLabel>

#include "searchedit.h"

DWIDGET_USE_NAMESPACE

class TitlebarWidgetPrivate
{
public:
    TitlebarWidgetPrivate(TitlebarWidget *parent) : q_ptr(parent) {}

    void fixSearchPosition();

    SearchEdit  *search = nullptr;

    TitlebarWidget *q_ptr;
    Q_DECLARE_PUBLIC(TitlebarWidget)
};

TitlebarWidget::TitlebarWidget(QWidget *parent) :
    DWidget(parent), d_ptr(new TitlebarWidgetPrivate(this))
{
    Q_D(TitlebarWidget);
    setFocusPolicy(Qt::NoFocus);
    setObjectName("TitlebarWidget");

    auto layout = new QHBoxLayout();
    //layout->setContentsMargins(0, 0, 0, 0);

    //auto leftWidget = new DWidget;
    // leftWidget->setObjectName("TitleLeft");
    // leftWidget->setFixedWidth(100);
    //auto leftLayout = new QHBoxLayout(leftWidget);
    //leftLayout->setSpacing(10);
    //leftLayout->setMargin(0);

    //auto iconLabel = new DLabel;
    //iconLabel->setObjectName("TitleIcon");
    //iconLabel->setFixedSize(32, 32);

    //leftLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
    //leftLayout->addStretch();

    d->search = new SearchEdit(this);
    d->search->setObjectName("TitleSearch");
//    d->search->setFixedSize(354, 40);
    d->search->setFixedWidth(354);
    d->search->setPlaceHolder(tr("Search"));
    d->search->clear();

    //auto rightWidget = new DWidget;
    // rightWidget->setObjectName("TitleLeft");
    //rightWidget->setFixedWidth(1);

    //layout->addWidget(leftWidget, 0,  Qt::AlignCenter);
    //layout->addStretch();
    layout->addWidget(d->search, Qt::AlignCenter);
    setLayout(layout);
    //layout->addStretch();
    //layout->addWidget(rightWidget, 0,  Qt::AlignCenter);

    connect(d->search, &SearchEdit::locateMusic, this, &TitlebarWidget::locateMusicInAllMusiclist);
    connect(d->search, &SearchEdit::searchText, this, &TitlebarWidget::search);
    connect(d->search, &SearchEdit::searchAborted, this, &TitlebarWidget::searchExited);
}

TitlebarWidget::~TitlebarWidget()
{
}

void TitlebarWidget::exitSearch()
{
    Q_D(TitlebarWidget);
    clearSearch();
}

void TitlebarWidget::clearSearch()
{
    Q_D(TitlebarWidget);
    d->search->clear();
    auto edit = d->search->findChild<QWidget *>("Edit");
    if (edit) {
        QApplication::postEvent(edit, new QFocusEvent(QEvent::FocusOut, Qt::MouseFocusReason));
    }
}

void TitlebarWidget::setSearchEnable(bool enable)
{
    Q_D(TitlebarWidget);
    d->search->setEnabled(enable);
}

void TitlebarWidget::setResultWidget(SearchResult *r)
{
    Q_D(TitlebarWidget);
    d->search->setResultWidget(r);
}

void TitlebarWidget::setViewname(const QString &viewname)
{
    Q_D(TitlebarWidget);
    d->search->setProperty("viewname", viewname);
}

void TitlebarWidget::selectPlaylist(PlaylistPtr playlistPtr)
{
    Q_D(TitlebarWidget);
    if (playlistPtr != d->search->curPlaylistPtr()) {
        d->search->selectPlaylist(playlistPtr);
        d->search->clear();
    }
}

void TitlebarWidget::resizeEvent(QResizeEvent *event)
{
//    Q_D(TitlebarWidget);
    DWidget::resizeEvent(event);
}

void TitlebarWidgetPrivate::fixSearchPosition()
{
    Q_Q(TitlebarWidget);
    //auto fixSize = QPoint(search->width() / 2, search->height() / 2);
    // auto fixPos = q->geometry().center() - fixSize;
    // search->setGeometry(fixPos.x(), fixPos.y(),
    //                    search->width(), search->height());

    //qDebug() << fixPos << search->size() << search->parent();
}
