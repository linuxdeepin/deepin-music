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

#include "playlistitem.h"

#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QStyle>
#include <QLineEdit>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QStyleFactory>

#include <ddialog.h>
#include <QMessageBox>
#include <DThemeManager>

#include "core/playlist.h"

#include "titeledit.h"

static int LineEditWidth = 105;

DWIDGET_USE_NAMESPACE

PlayListItem::PlayListItem(PlaylistPtr playlist, QWidget *parent) : QFrame(parent)
{
    m_data = playlist;
    Q_ASSERT(playlist);

    DThemeManager::instance()->registerWidget(this);

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

    m_titleedit = new TitelEdit;
    m_titleedit->setObjectName("PlayListTitle");
    m_titleedit->setFixedHeight(24);
    m_titleedit->setMaximumWidth(160);
    m_titleedit->setMaxLength(255);
    m_titleedit->setMaxLength(40);
    m_titleedit->setProperty("EditValue", playlist->displayName());

    m_titleedit->setDisabled(true);
    if (playlist->readonly()) {
        m_titleedit->setReadOnly(true);
    }

    if (playlist->editmode()) {
        m_titleedit->setEnabled(true);
        QTimer::singleShot(100, this, [ = ] {
            m_titleedit->setFocus();
            m_titleedit->setCursorPosition(0);
            m_titleedit->setSelection(0, m_titleedit->text().length());
        });
    }

    interLayout->addWidget(icon, 0, Qt::AlignCenter);
    interLayout->addSpacing(10);
    interLayout->addWidget(m_titleedit, 0, Qt::AlignRight);
    interLayout->addStretch();

    layout->addWidget(interFrame);

    setFixedHeight(56);
    setFixedWidth(220);

    interLayout->addSpacing(5);


    // TODO: wtf
    QFont font(m_titleedit->font());
    font.setPixelSize(12);
    QFontMetrics fm(font);
    m_titleedit->setText(fm.elidedText(QString(playlist->displayName()),
                                       Qt::ElideMiddle, LineEditWidth));

    connect(m_titleedit, &QLineEdit::editingFinished,
    this, [ = ] {
        if (m_titleedit->text().isEmpty())
        {
            m_titleedit->setText(m_titleedit->property("EditValue").toString());
        } else {
            emit this->rename(m_titleedit->text());
            m_titleedit->setProperty("EditValue", m_titleedit->text());
        }

        QFont font(m_titleedit->font());
        font.setPixelSize(12);
        QFontMetrics fm(font);
        m_titleedit->setText(fm.elidedText(QString(m_titleedit->text()),
        Qt::ElideMiddle, LineEditWidth));

        m_titleedit->setEnabled(false);
    });

    connect(m_titleedit, &QLineEdit::returnPressed,
    this, [ = ] {
        m_titleedit->blockSignals(true);
        this->setFocus();
        m_titleedit->blockSignals(false);
    });

    connect(this, &PlayListItem::rename,
            m_data.data(), &Playlist::setDisplayName);
    connect(this, &PlayListItem::remove,
            m_data.data(), &Playlist::removed);
}

void PlayListItem::setActive(bool active)
{
    QString prefix;
    if (active) {
        m_titleedit->setProperty("status", "active");
    } else {
        m_titleedit->setProperty("status", "");
    }
    this->style()->unpolish(m_titleedit);
    this->style()->polish(m_titleedit);
    this->update();
}

void PlayListItem::setPlay(bool isPaly)
{
    if (isPaly) {
    } else {
    }
}

void PlayListItem::mouseDoubleClickEvent(QMouseEvent *event)
{
//    QFrame::mouseDoubleClickEvent(event);

    auto lineeditMousePos = m_titleedit->mapFromParent(event->pos());
    if (!m_titleedit->rect().contains(lineeditMousePos)) {
        return;
    }

    if (m_titleedit->isReadOnly()) {
        return;
    }

    QTimer::singleShot(0, this, [ = ] {
//        m_titleedit->setEnabled(true);
//        m_titleedit->setFocus();
//        m_titleedit->setCursorPosition(m_titleedit->text().length());
    });
}

void PlayListItem::onDelete()
{
    QString message = QString(tr("Are you sure to delete this playlist?"));

    DDialog warnDlg(this);
    warnDlg.setIcon(QIcon(":/common/image/del_notify.svg"));
    warnDlg.setTextFormat(Qt::AutoText);
    warnDlg.setTitle(message);
    warnDlg.addButton(tr("Cancel"), false, Dtk::Widget::DDialog::ButtonNormal);
    warnDlg.addButton(tr("Delete"), true , Dtk::Widget::DDialog::ButtonWarning);

    if (0 == warnDlg.exec()) {
        return;
    }
    emit this->remove();
}

void PlayListItem::onRename()
{
    QTimer::singleShot(1, this, [ = ] {
        auto value = m_titleedit->property("EditValue").toString();
        qDebug() << value;
        m_titleedit->setText(value);
        m_titleedit->setEnabled(true);
        m_titleedit->setFocus();
        m_titleedit->setCursorPosition(0);
        m_titleedit->setSelection(0, m_titleedit->text().length());
    });
}

