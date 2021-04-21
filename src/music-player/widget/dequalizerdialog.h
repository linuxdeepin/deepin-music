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

#ifndef DEQUALIZERDIALOG_H
#define DEQUALIZERDIALOG_H

#include <DAbstractDialog>

#include <QToolTip>
#include <DTitlebar>
#include <DSwitchButton>
#include <DTabWidget>
#include <DComboBox>
#include <DSlider>
#include <DBlurEffectWidget>
#include <DFloatingMessage>
#include <DPushButton>
#include <DTitlebar>
#include <DLabel>

#include "../core/musicsettings.h"
#include "vlc/Equalizer.h"


DWIDGET_USE_NAMESPACE


class DequalizerDialog: public DWidget
{
    Q_OBJECT
public:
    explicit DequalizerDialog(QWidget *parent = Q_NULLPTR);
    ~DequalizerDialog();

    void initConnection();
    void showCustom();
    void showCurMode(int *Allbauds);

    void readConfig();
    void initUI();
    void SliderOneEntry(DSlider *slider, int value);

signals:
    void setEqualizerEnable(bool enabled);
    void setEqualizerIndex(int curIndex);
    void setEqualizerpre(int val);
    void setEqualizerbauds(int index, int val);

public Q_SLOTS:
    void checkedChanged(bool checked);
    void setDefaultClicked();

private:
    void selectSlider(QObject *obj, QString val);
    bool eventFilter(QObject *obj, QEvent *event);

private:

    DTitlebar   *m_titleBar            = nullptr;
    DSwitchButton *m_switchBtn         = nullptr;
    DComboBox      *m_comBox           = nullptr;
    DPushButton    *m_saveBtn           = nullptr;

    DLabel      *m_switchLabel         = nullptr;
    DLabel      *m_lbBaudPre          = nullptr;
    DLabel      *m_lbBaud60           = nullptr;
    DLabel      *m_lbBaud170          = nullptr;
    DLabel      *m_lbBaud310          = nullptr;
    DLabel      *m_lbBaud600          = nullptr;
    DLabel      *m_lbBaud1K           = nullptr;
    DLabel      *m_lbBaud3K           = nullptr;
    DLabel      *m_lbBaud6K           = nullptr;
    DLabel      *m_lbBaud12K          = nullptr;
    DLabel      *m_lbBaud14K          = nullptr;
    DLabel      *m_lbBaud16K          = nullptr;

    DSlider     *m_sliderPre           = nullptr;
    DSlider     *m_slider60            = nullptr;
    DSlider     *m_slider170           = nullptr;
    DSlider     *m_slider310           = nullptr;
    DSlider     *m_slider600           = nullptr;
    DSlider     *m_slider1K            = nullptr;
    DSlider     *m_slider3K            = nullptr;
    DSlider     *m_slider6K            = nullptr;
    DSlider     *m_slider12K           = nullptr;
    DSlider     *m_slider14K           = nullptr;
    DSlider     *m_slider16K           = nullptr;
    DPushButton *m_btnDefault          = nullptr;
    DFloatingMessage *m_saveMessage     = nullptr;

    QStringList    m_effectType;
    QList<int> m_baudList;

    bool    m_switchFlag               = false;
    bool    m_changeFlag                = false;
    int     m_curEffect                 = 0;

    int flat_bauds[11] = {12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int Classical_bauds[11] = {12, 0, 0, 0, 0, 0, 0, -7, -7, -7, -9};
    int Club_bauds[11] = {6, 0, 0, 8, 5, 5, 5, 3, 0, 0, 0};
    int Dance_bauds[11] = {5, 9, 7, 2, 0, 0, -5, -7, -7, 0, 0};
    int Full_bass_bauds[11] = {5, -8, 9, 9, 5, 1, -4, -8, -10, -11, -11};
    int Full_bass_and_treble_bauds[11] = {4, 7, 5, 0, -7, -4, 1, 8, 11, 12, 12};
    int Full_treble_bauds[11] = {3, -9, -9, -9, -4, 2, 11, 16, 16, 16, 16};
    int Headphones_bauds[11] = {4, 4, 11, 5, -3, -2, 1, 4, 9, 12, 14};
    int Large_Hall_bauds[11] = {5, 10, 10, 5, 5, 0, -4, -4, -4, 0, 0};
    int Live_bauds[11] = {7, -4, 0, 4, 5, 5, 5, 4, 2, 2, 2};
    int Party_bauds[11] = {6, 7, 7, 0, 0, 0, 0, 0, 0, 7, 7};
    int Pop_bauds[11] = {6, -1, 4, 7, 8, 5, 0, -2, -2, -1, -1};
    int Reggae_bauds[11] = {8, 0, 0, 0, -5, 0, 6, 6, 0, 0, 0};
    int Rock_bauds[11] = {5, 8, 4, -5, -8, -3, 4, 8, 11, 11, 11};
    int Ska_bauds[11] = {6, -2, -4, -4, 0, 4, 5, 8, 9, 11, 9};
    int Soft_bauds[11] = {5, 4, 1, 0, -2, 0, 4, 8, 9, 11, 12};
    int Soft_rock_bauds[11] = {7, 4, 4, 2, 0, -4, -5, -3, 0, 2, 8};
    int Techno_bauds[11] = {5, 8, 5, 0, -5, -4, 0, 8, 9, 9, 8};

    QList<int *> AllbaudTypes;
    //slidergroup
    DBlurEffectWidget *slWidget  = nullptr;
};

#endif // DEQUALIZERDIALOG_H
