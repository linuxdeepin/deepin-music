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

#pragma once

#include <QScopedPointer>
#include <DWidget>
#include <DSuggestButton>
#include <QLabel>

DWIDGET_USE_NAMESPACE

class AddMusicWidget : public DWidget
{
    Q_OBJECT
public:
    explicit AddMusicWidget(QWidget *parent = Q_NULLPTR);
    ~AddMusicWidget() override;

public slots:
    void setThemeType(int type);
    void slotAddMusicButtonClicked();
    void setSongList(const QString &hash);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private:
    QLabel                  *m_text = nullptr;
    DSuggestButton          *m_addMusicButton = nullptr;
    QLabel                  *m_logo = nullptr;
    QString                  m_currentHash = "all";
};

