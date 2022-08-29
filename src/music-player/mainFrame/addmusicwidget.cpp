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

#include "addmusicwidget.h"

#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QStandardPaths>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QButtonGroup>

#include <DFileDialog>
#include <DLabel>
#include <DPushButton>
#include <DHiDPIHelper>
#include <DPalette>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <DApplicationHelper>

#include "musicsettings.h"
#include "databaseservice.h"
#include "player.h"
#include "ac-desktop-define.h"
#include "commonservice.h"
DGUI_USE_NAMESPACE

AddMusicWidget::AddMusicWidget(QWidget *parent) : DWidget(parent)
{
    AC_SET_OBJECT_NAME(this, AC_addMusicWidget);
    AC_SET_ACCESSIBLE_NAME(this, AC_addMusicWidget);

    setAcceptDrops(true);
    setAutoFillBackground(true);

    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);

    m_logo = new QLabel(this);
    m_logo->setFixedSize(128, 128);
    m_logo->setObjectName("ImportViewLogo");
    m_logo->setPixmap(QIcon::fromTheme("import_music_light").pixmap(QSize(128, 128)));

    //将button添加到Group有非Check状态
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);

    m_addMusicButton = new DSuggestButton(this);
    m_addMusicButton->setFixedSize(302, 36);
    m_addMusicButton->setText(tr("Add Music"));
    m_addMusicButton->setFocusPolicy(Qt::TabFocus);
    m_addMusicButton->setDefault(true);
//    m_addMusicButton->setCheckable(true);
//    m_addMusicButton->setChecked(true);
    DFontSizeManager::instance()->bind(m_addMusicButton, DFontSizeManager::T6, QFont::Normal);
    buttonGroup->addButton(m_addMusicButton);

    m_text = new QLabel(this);
    m_text->setObjectName("ImportViewText");
    m_text->setText(tr("Or drag music files here"));
    DFontSizeManager::instance()->bind(m_text, DFontSizeManager::T8, QFont::Normal);

    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(m_logo, 0, Qt::AlignCenter);
    layout->addSpacing(12);
    layout->addWidget(m_addMusicButton, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(m_text, 0, Qt::AlignCenter);
    layout->addStretch();

    AC_SET_OBJECT_NAME(m_addMusicButton, AC_addMusicWidgetButton);
    AC_SET_ACCESSIBLE_NAME(m_addMusicButton, AC_addMusicWidgetButton);

    connect(m_addMusicButton, &DPushButton::clicked, this,  &AddMusicWidget::slotAddMusicButtonClicked);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &AddMusicWidget::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());
}

AddMusicWidget::~AddMusicWidget()
{

}

void AddMusicWidget::setThemeType(int type)
{
    // 浅色
    if (type == 1) {
        m_logo->setPixmap(QIcon::fromTheme("import_music_light").pixmap(QSize(128, 128)));
    } else { // 深色
        m_logo->setPixmap(QIcon::fromTheme("import_music_dark").pixmap(QSize(128, 128)));
    }
}

void AddMusicWidget::slotAddMusicButtonClicked()
{
    // 默认导入到所有音乐中
    emit CommonService::getInstance()->signalAddMusic();
}

void AddMusicWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return;
    }

    DWidget::dragEnterEvent(event);
}

void AddMusicWidget::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("text/uri-list")) {
        return;
    }

    auto urls = event->mimeData()->urls();
    QStringList localpaths;
    for (auto &url : urls) {
        localpaths << (url.isLocalFile() ? url.toLocalFile() : url.path());
    }

    if (!localpaths.isEmpty()) {
        DataBaseService::getInstance()->importMedias(m_currentHash, localpaths);
    }
}

void AddMusicWidget::setSongList(const QString &hash)
{
    m_currentHash = hash;
}
