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

#include "dequalizerdialog.h"

#include <QPainter>
#include <QProxyStyle>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QToolTip>

#include <DAbstractDialog>
#include <DFontSizeManager>
#include <DFrame>
#include <DLabel>
#include <DTitlebar>
#include <DSwitchButton>
#include <DTabWidget>
#include <DComboBox>
#include <DSlider>
#include <DBlurEffectWidget>
#include <DFloatingMessage>
#include <DPushButton>
#include <DTitlebar>
#include <DApplicationHelper>

#include <vlc/Equalizer.h>
#include <vlc/MediaPlayer.h>
#include "ac-desktop-define.h"
#include "player.h"

DGUI_USE_NAMESPACE

const int LableMaximumWidth = 50;

//开机后默认参数设置
void DequalizerDialog::readConfig()
{
//    Q_Q(DequalizerDialog);
    m_switchFlag = MusicSettings::value("equalizer.all.switch").toBool();
    m_curEffect   = MusicSettings::value("equalizer.all.curEffect").toInt();

}

void DequalizerDialog::initUI()
{
    this->setAutoFillBackground(true);

    QFont font;
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Normal);
    DequalizerDialog::tr("Equalizer");

    auto vlay  = new QVBoxLayout(this);
    auto hlay  = new QHBoxLayout;

    m_switchLabel = new DLabel(this);
    m_switchLabel->resize(14, 20);
    if (m_switchFlag) {
        m_switchLabel->setText(DequalizerDialog::tr("ON"));
    } else {
        m_switchLabel->setText(DequalizerDialog::tr("OFF"));
    }

    m_switchBtn = new DSwitchButton(this);
    m_switchBtn->resize(50, 20);
    AC_SET_OBJECT_NAME(m_switchBtn, AC_switchBtn);
    AC_SET_ACCESSIBLE_NAME(m_switchBtn, AC_switchBtn);
//    //设置底色
//    DPalette sw;
//    sw.setColor(DPalette::Button,QColor("#000000"));
//    mswitchBtn->setPalette(sw);

    m_comBox = new DComboBox(this);
    m_comBox->setMinimumSize(240, 36);
    m_comBox->addItems(m_effectType);
    AC_SET_OBJECT_NAME(m_comBox, AC_effectCombox);
    AC_SET_ACCESSIBLE_NAME(m_comBox, AC_effectCombox);


    m_saveBtn = new DPushButton(this);
    m_saveBtn->setMinimumSize(110, 36);
    m_saveBtn->setText(DequalizerDialog::tr("Save"));
    AC_SET_OBJECT_NAME(m_saveBtn, AC_saveBtn);
    AC_SET_ACCESSIBLE_NAME(m_saveBtn, AC_saveBtn);

    slWidget = new DBlurEffectWidget(this);
    QHBoxLayout *hlayout  = new QHBoxLayout;
    hlayout->setSpacing(5);
    QVector<QVBoxLayout *> allVlayouts;
    for (int i = 0; i < 11; ++i) {
        QVBoxLayout *vlayout0  = new QVBoxLayout;
        vlayout0->setSpacing(0);
        allVlayouts.append(vlayout0);
        hlayout->addLayout(vlayout0);
    }

    QVector<QString> acObjectNames;
    acObjectNames.append(AC_baud_pre);
    acObjectNames.append(AC_slider_60);
    acObjectNames.append(AC_slider_170);
    acObjectNames.append(AC_slider_310);
    acObjectNames.append(AC_slider_600);
    acObjectNames.append(AC_slider_1K);
    acObjectNames.append(AC_slider_3K);
    acObjectNames.append(AC_slider_6K);
    acObjectNames.append(AC_slider_12K);
    acObjectNames.append(AC_slider_14K);
    acObjectNames.append(AC_slider_16K);

    QVector<QString> sliderObjectNames;
    sliderObjectNames.append(tr("Preamplifier"));
    sliderObjectNames.append("60");
    sliderObjectNames.append("170");
    sliderObjectNames.append("310");
    sliderObjectNames.append("600");
    sliderObjectNames.append("1K");
    sliderObjectNames.append("3K");
    sliderObjectNames.append("6K");
    sliderObjectNames.append("12K");
    sliderObjectNames.append("14K");
    sliderObjectNames.append("16K");

    QVector<QHBoxLayout *> allValueLayouts;
    QVector<QHBoxLayout *> allSliderLayouts;
    QVector<DLabel *> allValueLabels;
    for (int i = 0; i < acObjectNames.size(); ++i) {
        DLabel *baudLabel = new DLabel(this);
        m_baudLabels.append(baudLabel);
        baudLabel->setMaximumWidth(LableMaximumWidth);
        baudLabel->setMinimumHeight(22);
        baudLabel->setAlignment(Qt::AlignCenter);
        auto hlayout_lb_baud_pre  = new QHBoxLayout;
        hlayout_lb_baud_pre->addStretch();
        hlayout_lb_baud_pre->addWidget(baudLabel);
        hlayout_lb_baud_pre->addStretch();
        allValueLayouts.append(hlayout_lb_baud_pre);

        DSlider *baudSlider = new DSlider(Qt::Vertical, this);
        m_allBaudSliders.append(baudSlider);
        AC_SET_OBJECT_NAME(baudSlider, acObjectNames[i]);
        AC_SET_ACCESSIBLE_NAME(baudSlider, acObjectNames[i]);
        baudSlider->setMinimum(-20);
        baudSlider->setMaximum(20);
        baudSlider->setPageStep(1);
        auto hlayout_slider_pre  = new QHBoxLayout;
        hlayout_slider_pre->addStretch();
        hlayout_slider_pre->addWidget(baudSlider);
        hlayout_slider_pre->addStretch();
        allSliderLayouts.append(hlayout_slider_pre);

        DLabel *lb_preamplifier = new DLabel(sliderObjectNames[i], this);
        allValueLabels.append(lb_preamplifier);
        lb_preamplifier->setAlignment(Qt::AlignCenter);
        DFontSizeManager::instance()->bind(lb_preamplifier, DFontSizeManager::T8, QFont::Normal);
        DFontSizeManager::instance()->bind(baudLabel, DFontSizeManager::T8, QFont::Normal);
    }

    m_btnDefault = new DPushButton(tr("Restore Defaults"), this);
    m_btnDefault->setMinimumSize(240, 36);
    AC_SET_OBJECT_NAME(m_btnDefault, AC_Restore);
    AC_SET_ACCESSIBLE_NAME(m_btnDefault, AC_Restore);

    for (int i = 0; i < allVlayouts.size(); ++i) {
        allVlayouts[i]->addLayout(allValueLayouts[i]);
        allVlayouts[i]->addLayout(allSliderLayouts[i], 100);
        allVlayouts[i]->addWidget(allValueLabels[i]);
    }

    slWidget->setBlurRectXRadius(8);
    slWidget->setBlurRectYRadius(8);
    slWidget->setRadius(8);
    slWidget->setBlurEnabled(true);
    slWidget->setMode(DBlurEffectWidget::GaussianBlur);
//    slWidget->setMinimumWidth(517);
    slWidget->setMinimumHeight(264);
    slWidget->setLayout(hlayout);
    slWidget->setContentsMargins(10, 4, 9, 18);
    AC_SET_OBJECT_NAME(slWidget, AC_slWidget);
    AC_SET_ACCESSIBLE_NAME(slWidget, AC_slWidget);

    //第一行
    hlay->addWidget(m_switchLabel, Qt::AlignLeft);
    hlay->addWidget(m_switchBtn, Qt::AlignLeft);
    hlay->addWidget(m_comBox, 20, Qt::AlignCenter);
    hlay->addWidget(m_saveBtn, Qt::AlignRight);

    //第二行
    vlay->addLayout(hlay);
    vlay->addSpacing(18);
    vlay->addWidget(slWidget, Qt::AlignLeft);
    vlay->addSpacing(18);
    vlay->addWidget(m_btnDefault, 0, Qt::AlignCenter);
    vlay->setMargin(0);
    this->setContentsMargins(0, 0, 0, 0);

    AllbaudTypes.append(flat_bauds);
    AllbaudTypes.append(Classical_bauds);
    AllbaudTypes.append(Club_bauds);
    AllbaudTypes.append(Dance_bauds);
    AllbaudTypes.append(Full_bass_bauds);
    AllbaudTypes.append(Full_bass_and_treble_bauds);
    AllbaudTypes.append(Full_treble_bauds);
    AllbaudTypes.append(Headphones_bauds);
    AllbaudTypes.append(Large_Hall_bauds);
    AllbaudTypes.append(Live_bauds);
    AllbaudTypes.append(Party_bauds);
    AllbaudTypes.append(Pop_bauds);
    AllbaudTypes.append(Reggae_bauds);
    AllbaudTypes.append(Rock_bauds);
    AllbaudTypes.append(Ska_bauds);
    AllbaudTypes.append(Soft_bauds);
    AllbaudTypes.append(Soft_rock_bauds);
    AllbaudTypes.append(Techno_bauds);

    m_switchBtn->setChecked(m_switchFlag);
    m_comBox->setEnabled(m_switchFlag);
    m_saveBtn->setEnabled(m_switchFlag);

    for (DSlider *slider : this->findChildren<DSlider *>()) {
        slider->setAttribute(Qt::WA_Hover, true); //开启悬停事件
        slider->installEventFilter(this);       //安装事件过滤器
        slider->setEnabled(m_switchFlag);
    }
    if (m_curEffect != 0) {
        this->showCurMode(AllbaudTypes.at(m_curEffect - 1));
        m_comBox->setCurrentIndex(m_curEffect);
    } else {
        this->showCustom();
    }
}

void DequalizerDialog::SliderOneEntry(DSlider *slider, int value)
{
    this->selectSlider(slider, QString::number(value));
    for (int i = 0; i < m_allBaudSliders.size(); ++i) {
        //剔除其他信号
        if (slider != m_allBaudSliders[i])
            continue;
        if (i == 0) {
            Q_EMIT this->setEqualizerpre(value);
        } else {
            Q_EMIT this->setEqualizerbauds(i - 1, value);
        }
        break;
    }
}


DequalizerDialog::DequalizerDialog(QWidget *parent):
    DWidget(parent)
{
    AC_SET_OBJECT_NAME(this, AC_Dequalizer);
    AC_SET_ACCESSIBLE_NAME(this, AC_Dequalizer);
    m_effectType << DequalizerDialog::tr("Custom")
                 << DequalizerDialog::tr("Monophony")
                 << DequalizerDialog::tr("Classical")
                 << DequalizerDialog::tr("Club")
                 << DequalizerDialog::tr("Dance")
                 << DequalizerDialog::tr("Full Bass")
                 << DequalizerDialog::tr("Full Bass and Treble")
                 << DequalizerDialog::tr("Full Treble")
                 << DequalizerDialog::tr("Headphones")
                 << DequalizerDialog::tr("Hall")
                 << DequalizerDialog::tr("Live")
                 << DequalizerDialog::tr("Party")
                 << DequalizerDialog::tr("Pop")
                 << DequalizerDialog::tr("Reggae")
                 << DequalizerDialog::tr("Rock")
                 << DequalizerDialog::tr("Ska")
                 << DequalizerDialog::tr("Soft")
                 << DequalizerDialog::tr("Soft Rock")
                 << DequalizerDialog::tr("Techno");
    this->readConfig();
    this->initUI();
    initConnection();
    QFont font;

// 解决字体不会根据系统字体大小改变问题
//    font.setPixelSize(13);
    QFontMetrics fm(font);
    QWidget *parentWidget = dynamic_cast<QWidget *>(this->parent());
    this->m_saveMessage  = new DFloatingMessage(DFloatingMessage::TransientType, parentWidget);
    this->m_saveMessage->setFont(font);
    this->m_saveMessage->setIcon(QIcon::fromTheme("notify_success"));
    this->m_saveMessage->setMessage(tr("Sound Effects Saved"));
    int Minwid = fm.width(tr("Sound Effects Saved")) + 70;
    this->m_saveMessage->setMinimumSize(Minwid, 60);
    this->m_saveMessage->setDuration(2000);
    if (parentWidget) {
        this->m_saveMessage->move(parentWidget->width() / 2 - 80, parentWidget->height() - 70);
    }
    this->m_saveMessage->hide();


    connect(this, &DequalizerDialog::setEqualizerEnable,
            Player::getInstance(), &Player::setEqualizerEnable);
    connect(this, &DequalizerDialog::setEqualizerpre,
            Player::getInstance(), &Player::setEqualizerpre);
    connect(this, &DequalizerDialog::setEqualizerbauds,
            Player::getInstance(), &Player::setEqualizerbauds);
    connect(this, &DequalizerDialog::setEqualizerIndex,
            Player::getInstance(), &Player::setEqualizerCurMode);
}

DequalizerDialog::~DequalizerDialog()
{
    delete this->m_saveMessage;
}

void DequalizerDialog::initConnection()
{
    connect(this->m_switchBtn, &DSwitchButton::checkedChanged, this, &DequalizerDialog::checkedChanged);

    for (DSlider *slider : findChildren<DSlider *>()) {
        connect(slider, &DSlider::sliderReleased, [ = ]() {
            this->m_changeFlag = true;
            if (this->m_comBox->currentIndex() != 0) {
                this->m_comBox->setCurrentIndex(0);
            }
            int value = slider->value();
            this->SliderOneEntry(slider, value);
        });

        connect(slider, &DSlider::valueChanged, [ = ](int value) {
            this->m_saveBtn->setEnabled(true);

            // 音乐模式切换到自定义模式，应该进行值的变化显示
            this->SliderOneEntry(slider, value);
        });
    }

    connect(this->m_saveBtn, &DPushButton::clicked, [ = ]() {
        for (DSlider *slider : findChildren<DSlider *>()) {
            MusicSettings::setOption("equalizer.all." + slider->objectName(), slider->value());
        }
        MusicSettings::setOption("equalizer.all.curEffect", 0);
        this->m_saveMessage->show();
    });

    connect(this->m_comBox, QOverload<int>::of(&DComboBox::currentIndexChanged),
    this, [ = ](int index) {
        if (index == 0) {
            showCustom();
        } else {
            this->m_changeFlag = false;
            showCurMode(this->AllbaudTypes.at(index - 1));
            Q_EMIT setEqualizerIndex(index);
            MusicSettings::setOption("equalizer.all.curEffect", index);
        }
    });
    connect(this->m_btnDefault, &DPushButton::clicked, this, &DequalizerDialog::setDefaultClicked);

}
//显示自定义模式
void DequalizerDialog::showCustom()
{
    if (this->m_changeFlag) {
        return;
    }

    for (DSlider *slider : findChildren<DSlider *>()) {
        int indexbaud = MusicSettings::value("equalizer.all." + slider->objectName()).toInt();
        slider->setValue(indexbaud);
        selectSlider(slider, "");
    }
    MusicSettings::setOption("equalizer.all.curEffect", 0);
}

//显示非自定义模式
void DequalizerDialog::showCurMode(int *Allbauds)
{
    if (Allbauds == nullptr) {
        return;
    }
    int i = 0;
    for (DSlider *slider : findChildren<DSlider *>()) {
        if (this->m_switchBtn->isChecked()) {
            slider->setEnabled(true);
        }
        slider->setValue(Allbauds[i]);
        selectSlider(slider, "");
        i++;
    }
    this->m_saveBtn->setEnabled(false);
}

//开关
void DequalizerDialog::checkedChanged(bool checked)
{
    for (DSlider *slider : findChildren<DSlider *>()) {
        slider->setEnabled(checked);
    }
    this->m_switchLabel->setText(tr("OFF"));
    if (checked) {
        this->m_switchLabel->setText(tr("ON"));
        //flat;
        showCurMode(this->flat_bauds);
        this->m_comBox->setCurrentIndex(1);
        Q_EMIT setEqualizerIndex(1);
        MusicSettings::setOption("equalizer.all.curEffect", 1);
    } else {
        Q_EMIT setEqualizerIndex(-1);
        MusicSettings::setOption("equalizer.all.curEffect", 1);
    }
    this->m_comBox->setEnabled(checked);
    this->m_saveBtn->setEnabled(false);
//    Q_EMIT setEqualizerEnable(checked);
    MusicSettings::setOption("equalizer.all.switch", checked);
}

//恢复默认
void DequalizerDialog::setDefaultClicked()
{
    if (this->m_switchBtn->isChecked()) {
        showCurMode(this->flat_bauds);
        this->m_switchBtn->setChecked(false);
        this->m_comBox->setCurrentIndex(1);
        MusicSettings::setOption("equalizer.all.curEffect", 1);
        for (DSlider *slider : findChildren<DSlider *>()) {
            if (slider == m_allBaudSliders[0]) {
                MusicSettings::setOption("equalizer.all." + slider->objectName(), 12);
            } else {
                MusicSettings::setOption("equalizer.all." + slider->objectName(), 0);
            }
        }
    }
}
void DequalizerDialog::selectSlider(QObject *obj, QString SliderVal)
{
    for (int i = 0; i < m_allBaudSliders.size(); ++i) {
        //剔除其他信号
        if (obj != m_allBaudSliders[i])
            continue;
        m_baudLabels[i]->setText(SliderVal);
        break;
    }
}

bool DequalizerDialog::eventFilter(QObject *obj, QEvent *event)
{
    for (DSlider *slider : findChildren<DSlider *>()) {
        if (obj == slider) {
            if (event->type() == QEvent::HoverEnter) {
                selectSlider(slider, QString::number(slider->value()));
                return true;
            }
            if (event->type() == QEvent::HoverLeave) {
                selectSlider(slider, "");
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
