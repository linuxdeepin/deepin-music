/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
#include <thememanager.h>

#include "core/playlist.h"

#include "titeledit.h"

static int LineEditWidth = 105;

DWIDGET_USE_NAMESPACE

PlayListItem::PlayListItem(PlaylistPtr playlist, QWidget *parent) : QFrame(parent)
{
    m_data = playlist;
    Q_ASSERT(playlist);

    ThemeManager::instance()->regisetrWidget(this);

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

    playingAnimation = new Dtk::Widget::DPictureSequenceView(this);
    playingAnimation->setFixedSize(17, 13);
    interLayout->addWidget(playingAnimation);
    interLayout->addSpacing(5);
    playingAnimation->hide();

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
        prefix = highlightAnimationPrefix();
        m_titleedit->setProperty("status", "active");
    } else {
        prefix = animationPrefix();
        m_titleedit->setProperty("status", "");
    }

    auto activePrefix = playingAnimation->property("ActivePrefix").toString();
    if (activePrefix != prefix) {
        QStringList urls;
        auto urlTemp = QString("%1/%2.png").arg(prefix);
        for (int i = 0; i < 94; ++i) {
            urls << urlTemp.arg(i);
        }
        playingAnimation->setPictureSequence(urls);
        playingAnimation->setProperty("ActivePrefix", prefix);
        playingAnimation->hide();
    }
    this->style()->unpolish(m_titleedit);
    this->style()->polish(m_titleedit);
    this->update();
}

void PlayListItem::setPlay(bool isPaly)
{
    if (isPaly) {
        playingAnimation->show();
        playingAnimation->pause();
    } else {
        playingAnimation->hide();
        playingAnimation->pause();
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
    warnDlg.setIcon(QIcon(":/common/image/del_notify.png"));
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

