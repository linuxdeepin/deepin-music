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

#include <DDialog>
#include <DFrame>
#include <mediameta.h>

DWIDGET_USE_NAMESPACE

class InfoDialog;
class MusicImageButton;
class MusicTitleImageWidget;
class PlayListView;

#include <DLabel>
#include <DPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <DIconButton>

class SubSonglistWidget : public DWidget
{
    Q_OBJECT

public:
    explicit SubSonglistWidget(const QString &hash, QWidget *parent = Q_NULLPTR);
    ~SubSonglistWidget();

    void setThemeType(int type);

    // 更新Dialog UI信息
    void flushDialog(QMap<QString, MediaMeta> musicinfos, ListPageSwitchType listPageType);
public slots:
    void slotPlayAllClicked();
    void slotPlayRandomClicked();

private:
    void setTitleImage(QPixmap &img);
    void initUI();
protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
private:
    // 当前主题
    int m_themeType = 1;
    // 专辑或则演唱者的hash
    QString                             m_hash;

    QLabel                             *m_titleImage          = nullptr;
    QString                             m_title;
    DLabel                             *m_titleLabel          = nullptr;
    DLabel                             *m_infoLabel           = nullptr;
    DPushButton                        *m_btPlayAll           = nullptr;
    DPushButton                        *m_btRandomPlay        = nullptr;
    InfoDialog                         *m_infoDialog          = nullptr;
    PlayListView                       *m_musicListInfoView   = nullptr;
    QPixmap                             m_img;
};
