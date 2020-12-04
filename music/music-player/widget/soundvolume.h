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

#include <QScopedPointer>
#include <QPainter>
#include <DFrame>
#include <DLabel>
#include <DSlider>
#include <DToolButton>

class SoundPixmapButton;

using namespace Dtk::Widget;

class SoundVolume : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int radius READ radius WRITE setRadius)
    Q_PROPERTY(QBrush background READ background WRITE setBackground)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
public:
    explicit SoundVolume(QWidget *parent = nullptr);
    ~SoundVolume() override;

    int volume() const;
    int radius() const;
    QColor borderColor() const;
    QBrush background() const;

    void setVolumeFromExternal(int vol);
signals:
    void delayAutoHide();

public slots:
    void delayHide();
    void setBackground(QBrush background);
    void setRadius(int radius);
    void setBorderColor(QColor borderColor);
    void volumeIcon();
    void syncMute(bool mute);
    void slotTheme(int type);
    void slotSoundClick();
protected:
    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    virtual void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    virtual void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private slots:
    void slotSetVolume(int volume);
private:
//    QScopedPointer<SoundVolumePrivate> d_ptr;


    DLabel      *m_lblPersent = nullptr;
    DSlider     *m_volSlider           = nullptr;
//    SoundPixmapButton *m_btSound         = nullptr;
    DToolButton       *m_btSound         = nullptr;

    QBrush      m_background;
    QColor      m_borderColor = QColor(0, 0, 0,  255 * 2 / 10);

    int         m_radius      = 20;
    bool        m_mouseIn     = false;
    int         m_sThemeType  = 0;
    bool        m_volMute     = false;

};






