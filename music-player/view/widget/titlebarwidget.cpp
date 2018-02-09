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
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QApplication>
#include <QFocusEvent>
#include <QGraphicsOpacityEffect>

#include <DWidgetUtil>
#include <DThemeManager>
#include <dsearchedit.h>

#include "searchedit.h"

DWIDGET_USE_NAMESPACE

class TitlebarWidgetPrivate
{
public:
    TitlebarWidgetPrivate(TitlebarWidget *parent) : q_ptr(parent) {}

    void fixSearchPosition();

    SearchEdit  *search = nullptr;
    QPushButton *btBack = nullptr;

    TitlebarWidget *q_ptr;
    Q_DECLARE_PUBLIC(TitlebarWidget)
};

TitlebarWidget::TitlebarWidget(QWidget *parent) :
    QFrame(parent), d_ptr(new TitlebarWidgetPrivate(this))
{
    Q_D(TitlebarWidget);
    setFocusPolicy(Qt::NoFocus);
    setObjectName("TitlebarWidget");

    DThemeManager::instance()->registerWidget(this, QStringList({"viewname"}));

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 10, 5);

    auto leftWidget = new QFrame;
    leftWidget->setObjectName("TitleLeft");
    leftWidget->setFixedWidth(148);
    auto leftLayout = new QHBoxLayout(leftWidget);
    leftLayout->setSpacing(10);
    leftLayout->setMargin(0);

    auto iconLabel = new QLabel;
    iconLabel->setObjectName("TitleIcon");
    iconLabel->setFixedSize(24, 24);

    d->btBack = new QPushButton;
    d->btBack->setObjectName("TitleBack");
    d->btBack->setFixedSize(24, 24);
    d->btBack->hide();

    leftLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
    leftLayout->addWidget(d->btBack, 0, Qt::AlignCenter);
    leftLayout->addStretch();

    d->search = new SearchEdit(this);
    d->search->setObjectName("TitleSearch");
    d->search->setFixedSize(278, 26);
    d->search->setPlaceHolder(tr("Search"));
    d->search->clear();

    auto rightWidget = new QFrame;
    rightWidget->setObjectName("TitleLeft");
    rightWidget->setFixedWidth(1);

    layout->addWidget(leftWidget, 0,  Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(d->search, 0,  Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(rightWidget, 0,  Qt::AlignCenter);

    connect(d->search, &SearchEdit::locateMusic, this, &TitlebarWidget::locateMusicInAllMusiclist);
    connect(d->search, &SearchEdit::searchText, this, &TitlebarWidget::search);
    connect(d->btBack, &QPushButton::clicked, this, &TitlebarWidget::searchExited);

    connect(this, &TitlebarWidget::search, this, [ = ]() {
        d->btBack->show();
//        d->fixSearchPosition();
    });
    connect(this, &TitlebarWidget::searchExited, this, [ = ]() {
        d->btBack->hide();
        clearSearch();
//        d->fixSearchPosition();
    });
}

TitlebarWidget::~TitlebarWidget()
{
}

void TitlebarWidget::exitSearch()
{
    Q_D(TitlebarWidget);
    d->btBack->hide();
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

void TitlebarWidget::resizeEvent(QResizeEvent *event)
{
//    Q_D(TitlebarWidget);
    QFrame::resizeEvent(event);
}

void TitlebarWidgetPrivate::fixSearchPosition()
{
    Q_Q(TitlebarWidget);
    auto fixSize = QPoint(search->width() / 2, search->height() / 2);
    auto fixPos = q->geometry().center() - fixSize;
    search->setGeometry(fixPos.x(), fixPos.y(),
                        search->width(), search->height());

    qDebug() << fixPos << search->size() << search->parent();
}
