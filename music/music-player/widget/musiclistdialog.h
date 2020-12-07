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

#pragma once

#include <DDialog>
#include <mediameta.h>

DWIDGET_USE_NAMESPACE

class InfoDialog;
class MusicImageButton;
class MusicTitleImageWidget;
class MusicListInfoView;

#include <DLabel>
#include <DPushButton>
#include <QVBoxLayout>
#include <QGridLayout>



class MusicListDialog : public DAbstractDialog
{
    Q_OBJECT

public:
    explicit MusicListDialog(const QString &hash, QWidget *parent = Q_NULLPTR);
    ~MusicListDialog();

    void setThemeType(int type);

    // 更新Dialog UI信息
    void flushDialog(QMap<QString, MediaMeta> musicinfos, int isAlbumDialog);
public slots:
    void slotPlayAllClicked();
    void slotPlayRandomClicked();

private:
    void setTitleImage(QPixmap &img);
    void initUI();
private:
    // 当前主题
    int m_themeType = 1;
    // 专辑或则演唱者的hash
    QString hash;

    DLabel                             *m_titleImage          = nullptr;
    DLabel                             *m_titleLabel          = nullptr;
    DLabel                             *m_infoLabel           = nullptr;
    DPushButton                        *m_btPlayAll           = nullptr;
    DPushButton                        *m_btRandomPlay        = nullptr;
    InfoDialog                         *m_infoDialog          = nullptr;
    MusicImageButton                   *m_closeBt             = nullptr;
    MusicListInfoView                  *m_musicListInfoView   = nullptr;
};

