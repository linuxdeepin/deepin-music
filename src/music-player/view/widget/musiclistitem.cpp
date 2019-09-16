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

#include "musiclistitem.h"

#include <QDebug>
#include <QTimer>
#include <QMouseEvent>
#include <QHBoxLayout>

#include <DLineEdit>
#include <DLabel>
#include <DMenu>

#include <ddialog.h>
#include <QMessageBox>

#include "core/playlist.h"

static int LineEditWidth = 105;

MusicListItem::MusicListItem(PlaylistPtr playlist, QWidget *parent) : DFrame(parent)
{
    m_data = playlist;
    Q_ASSERT(playlist);

    setObjectName("MusicListItem");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(0);

    auto interFrame = new DFrame;
    interFrame->setObjectName("MusicListInterFrame");

    auto interLayout = new QHBoxLayout(interFrame);
    interLayout->setContentsMargins(0, 0, 0, 0);
    interLayout->setSpacing(0);

    auto icon = new DLabel;
    icon->setObjectName("MusicListIcon");
    icon->setFixedSize(24, 24);
    icon->setProperty("iconName", playlist->icon());

    m_titleedit = new DLineEdit;
    m_titleedit->setObjectName("MusicListTitle");
    m_titleedit->setFixedHeight(24);
    m_titleedit->setMaximumWidth(160);
    m_titleedit->lineEdit()->setMaxLength(255);
    m_titleedit->lineEdit()->setMaxLength(40);
    m_titleedit->setProperty("EditValue", playlist->displayName());

    m_titleedit->setDisabled(true);
    if (playlist->readonly()) {
        m_titleedit->lineEdit()->setReadOnly(true);
    }

    if (playlist->editmode()) {
        m_titleedit->setEnabled(true);
        QTimer::singleShot(200, this, [ = ] {
            m_titleedit->setFocus();
            m_titleedit->lineEdit()->setCursorPosition(0);
            m_titleedit->lineEdit()->setSelection(0, m_titleedit->text().length());
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

    connect(m_titleedit, &DLineEdit::editingFinished,
    this, [ = ] {
        if (m_titleedit->text().isEmpty())
        {
            m_titleedit->setText(m_titleedit->property("EditValue").toString());
        } else
        {
            Q_EMIT this->rename(m_titleedit->text());
            m_titleedit->setProperty("EditValue", m_titleedit->text());
        }

        QFont font(m_titleedit->font());
        font.setPixelSize(12);
        QFontMetrics fm(font);
        m_titleedit->setText(fm.elidedText(QString(m_titleedit->text()),
                                           Qt::ElideMiddle, LineEditWidth));

        m_titleedit->setEnabled(false);
    });

    connect(m_titleedit, &DLineEdit::returnPressed,
    this, [ = ] {
        m_titleedit->blockSignals(true);
        this->setFocus();
        m_titleedit->blockSignals(false);
    });

    connect(this, &MusicListItem::rename,
            m_data.data(), &Playlist::setDisplayName);
    connect(this, &MusicListItem::remove,
            m_data.data(), &Playlist::removed);
}

void MusicListItem::setActive(bool active)
{
    QString prefix;
    if (active) {
        m_titleedit->setProperty("status", "active");
    } else {
        m_titleedit->setProperty("status", "");
    }
    this->update();
}

void MusicListItem::setPlay(bool isPaly)
{
    if (isPaly) {
    } else {
    }
}

void MusicListItem::mouseDoubleClickEvent(QMouseEvent *event)
{
//    DFrame::mouseDoubleClickEvent(event);

    auto lineeditMousePos = m_titleedit->mapFromParent(event->pos());
    if (!m_titleedit->rect().contains(lineeditMousePos)) {
        return;
    }

    if (m_titleedit->lineEdit()->isReadOnly()) {
        return;
    }

    QTimer::singleShot(0, this, [ = ] {
//        m_titleedit->setEnabled(true);
//        m_titleedit->setFocus();
//        m_titleedit->setCursorPosition(m_titleedit->text().length());
    });
}

void MusicListItem::onDelete()
{
    QString message = QString(tr("Are you sure you want to delete this playlist?"));

    DDialog warnDlg(this);
    warnDlg.setIcon(QIcon(":/common/image/del_notify.svg"));
    warnDlg.setTextFormat(Qt::AutoText);
    warnDlg.setTitle(message);
    warnDlg.addButton(tr("Cancel"), false, Dtk::Widget::DDialog::ButtonNormal);
    warnDlg.addButton(tr("Delete"), true, Dtk::Widget::DDialog::ButtonWarning);

    if (0 == warnDlg.exec()) {
        return;
    }
    Q_EMIT this->remove();
}

void MusicListItem::onRename()
{
    QTimer::singleShot(1, this, [ = ] {
        auto value = m_titleedit->property("EditValue").toString();
        qDebug() << value;
        m_titleedit->setText(value);
        m_titleedit->setEnabled(true);
        m_titleedit->setFocus();
        m_titleedit->lineEdit()->setCursorPosition(0);
        m_titleedit->lineEdit()->setSelection(0, m_titleedit->text().length());
    });
}

