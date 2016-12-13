/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "playlistitem.h"

#include <QTimer>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QMenu>

#include <ddialog.h>
#include <QMessageBox>
#include <dthememanager.h>

#include "../core/playlist.h"

DWIDGET_USE_NAMESPACE

PlayListItem::PlayListItem(PlaylistPtr playlist, QWidget *parent) : QFrame(parent)
{
    m_data = playlist;
    Q_ASSERT(playlist);

    setObjectName("PlayListItem");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(0);

    auto interFrame = new QFrame;
    interFrame->setObjectName("PlayListInterFrame");

    auto interLayout = new QHBoxLayout(interFrame);
    interLayout->setContentsMargins(0, 0, 0, 0);
    interLayout->setSpacing(0);

    auto icon = new QLabel;
    icon->setObjectName("PlayListIcon");
    icon->setFixedSize(48, 48);
    icon->setProperty("iconName", playlist->icon());

    m_titleedit = new QLineEdit;
    m_titleedit->setObjectName("PlayListTitle");
    m_titleedit->setFixedHeight(20);
    m_titleedit->setMinimumWidth(140);
    m_titleedit->setText(playlist->displayName());
    m_titleedit->setProperty("HistoryValue", m_titleedit->text());

    m_titleedit->setDisabled(true);
    if (playlist->readonly()) {
        m_titleedit->setReadOnly(true);
    }

    if (playlist->editmode()) {
        // TODO: for qt5.3
        m_titleedit->setEnabled(true);
        QTimer::singleShot(0, this, [ = ] {
            m_titleedit->setFocus();
            m_titleedit->setCursorPosition(0);
            m_titleedit->setSelection(0, m_titleedit->text().length());
        });
    }

    interLayout->addWidget(icon, 0, Qt::AlignCenter);
    interLayout->addSpacing(10);

    QSizePolicy spTitle(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spTitle.setHorizontalStretch(100);
    m_titleedit->setSizePolicy(spTitle);

    interLayout->addWidget(m_titleedit, 0, Qt::AlignRight);

    layout->addWidget(interFrame);

    setFixedHeight(56);
    setFixedWidth(220);

    D_THEME_INIT_WIDGET(PlayListItem);

    connect(m_titleedit, &QLineEdit::editingFinished,
    this, [ = ] {
        if (m_titleedit->text().isEmpty())
        {
            m_titleedit->setText(m_titleedit->property("HistoryValue").toString());
        } else {
            emit this->rename(m_titleedit->text());
            m_titleedit->setProperty("HistoryValue", m_titleedit->text());
        }

        m_titleedit->setEnabled(false);
    });
    connect(m_titleedit, &QLineEdit::returnPressed,
    this, [ = ] {
        m_titleedit->blockSignals(true);
        this->setFocus();
        m_titleedit->blockSignals(false);
    });

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &PlayListItem::customContextMenuRequested,
            this, &PlayListItem::showContextMenu);

    connect(this, &PlayListItem::rename,
            m_data.data(), &Playlist::setDisplayName);
    connect(this, &PlayListItem::remove,
            m_data.data(), &Playlist::removed);
}

void PlayListItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    QFrame::mouseDoubleClickEvent(event);

    auto lineeditMousePos = m_titleedit->mapFromParent(event->pos());
    if (!m_titleedit->rect().contains(lineeditMousePos)) {
        return;
    }

    if (m_titleedit->isReadOnly()) {
        return;
    }

    QTimer::singleShot(0, this, [ = ] {
        m_titleedit->setEnabled(true);
        m_titleedit->setFocus();
        m_titleedit->setCursorPosition(m_titleedit->text().length());
    });
}


void PlayListItem::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = this->mapToGlobal(pos);

    QMenu menu;
    auto playact = menu.addAction(tr("Play"));
    playact->setDisabled(0 == m_data->length());

    if (m_data->id() != AllMusicListID && m_data->id() != FavMusicListID) {
        menu.addAction(tr("Rename"));
        menu.addAction(tr("Delete"));
    }

    connect(&menu, &QMenu::triggered, this, [ = ](QAction * action) {
        if (action->text() == "Play") {
            emit this->playall(m_data);
        }
        if (action->text() == "Rename") {
            QTimer::singleShot(0, this, [ = ] {
                m_titleedit->setEnabled(true);
                m_titleedit->setFocus();
                m_titleedit->setCursorPosition(0);
                m_titleedit->setSelection(0, m_titleedit->text().length());
            });
        }

        if (action->text() == "Delete") {
            QString message = QString(tr("\nAre you sure to delete \"%1\"?")).arg(m_titleedit->text());

            DDialog warnDlg;
            warnDlg.setIcon(QIcon(":/image/notify_fail.png"));
            warnDlg.setTextFormat(Qt::AutoText);
            warnDlg.setMessage(message);
            warnDlg.addButtons(QStringList() << tr("Cancel") << tr("Delete"));
            if (0 == warnDlg.exec()) {
                return;
            }
            emit this->remove();
        }
    });

    menu.exec(globalPos);
}
