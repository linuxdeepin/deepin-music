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

#include <vlc/Equalizer.h>
#include <vlc/MediaPlayer.h>
#include "ac-desktop-define.h"

DGUI_USE_NAMESPACE

class CustomTabStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (type == QStyle::CT_TabBarTab) {
            s.transpose();
            s.rwidth() = 144; // 设置每个tabBar中item的大小
            s.rheight() = 30;
        }
        return s;
    }

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
        if (element == CE_TabBarTabLabel) {
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {

                if (tab->state & QStyle::State_Selected) {
                    painter->save();
                    painter->setPen(0x89cfff);
                    painter->setBrush(QBrush(0x89cfff));
                    painter->restore();
                }
                QTextOption option;
                option.setAlignment(Qt::AlignCenter);
                if (tab->state & QStyle::State_Selected) {
                    painter->setPen(0xf8fcff);
                } else {
                    painter->setPen(0x5d5d5d);
                }

                painter->drawText(tab->rect, tab->text, option);
                return;
            }
        }

        if (element == CE_TabBarTab) {
            QProxyStyle::drawControl(element, option, painter, widget);
        }
    }
};

//开机后默认参数设置
void DequalizerDialog::readConfig()
{
//    Q_Q(DequalizerDialog);
    m_switchFlag = MusicSettings::value("equalizer.all.switch").toBool();
    m_curEffect   = MusicSettings::value("equalizer.all.curEffect").toInt();

}

void DequalizerDialog::initUI()
{
    resize(720, 463);

    QFont font;
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Normal);

// 解决字体不会根据系统字体大小改变问题
//    font.setPixelSize(17);
    m_titleLabel  = new DLabel(DequalizerDialog::tr("Equalizer"));
    m_titleLabel->resize(51, 25);
    m_titleLabel->setFont(font);

    m_switchLabel = new DLabel;
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

    m_lbBaudPre = new DLabel(this);
    m_lbBaudPre->setMaximumWidth(20);
    m_lbBaudPre->setMinimumHeight(22);
    m_lbBaudPre->setAlignment(Qt::AlignCenter);
    auto hlayout_lb_baud_pre  = new QHBoxLayout;
    hlayout_lb_baud_pre->addStretch();
    hlayout_lb_baud_pre->addWidget(m_lbBaudPre);
    hlayout_lb_baud_pre->addStretch();
    m_sliderPre = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(m_sliderPre, AC_baud_pre);
    AC_SET_ACCESSIBLE_NAME(m_sliderPre, AC_baud_pre);
    m_sliderPre->setMinimum(-20);
    m_sliderPre->setMaximum(20);
    m_sliderPre->setPageStep(1);
    auto hlayout_slider_pre  = new QHBoxLayout;
    hlayout_slider_pre->addStretch();
    hlayout_slider_pre->addWidget(m_sliderPre);
    hlayout_slider_pre->addStretch();
    DLabel *lb_preamplifier = new DLabel(DequalizerDialog::tr("Preamplifier"));
    lb_preamplifier->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(lb_preamplifier, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(m_lbBaudPre, DFontSizeManager::T8, QFont::Normal);

    m_lbBaud60 = new DLabel(this);
    m_lbBaud60->setMaximumWidth(20);
    m_lbBaud60->setMinimumHeight(22);
    m_lbBaud60->setAlignment(Qt::AlignCenter);
    auto hlayout_lb_baud_60  = new QHBoxLayout;
    hlayout_lb_baud_60->addStretch();
    hlayout_lb_baud_60->addWidget(m_lbBaud60);
    hlayout_lb_baud_60->addStretch();
    m_slider60 = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(m_slider60, AC_slider_60);
    AC_SET_ACCESSIBLE_NAME(m_slider60, AC_slider_60);
    m_slider60->setMinimum(-20);
    m_slider60->setMaximum(20);
    m_slider60->setPageStep(1);
    auto hlayout_slider_60  = new QHBoxLayout;
    hlayout_slider_60->addStretch();
    hlayout_slider_60->addWidget(m_slider60);
    hlayout_slider_60->addStretch();
    DLabel *lb_60 = new DLabel("60");
    lb_60->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_lbBaud60, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(lb_60, DFontSizeManager::T8, QFont::Normal);

    m_lbBaud170 = new DLabel(this);
    m_lbBaud170->setMaximumWidth(20);
    m_lbBaud170->setMinimumHeight(22);
    m_lbBaud170->setAlignment(Qt::AlignCenter);
    auto hlayout_lb_baud_170  = new QHBoxLayout;
    hlayout_lb_baud_170->addStretch();
    hlayout_lb_baud_170->addWidget(m_lbBaud170);
    hlayout_lb_baud_170->addStretch();
    m_slider170 = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(m_slider170, AC_slider_170);
    AC_SET_ACCESSIBLE_NAME(m_slider170, AC_slider_170);
    m_slider170->setMinimum(-20);
    m_slider170->setMaximum(20);
    m_slider170->setPageStep(1);
    auto hlayout_slider_170  = new QHBoxLayout;
    hlayout_slider_170->addStretch();
    hlayout_slider_170->addWidget(m_slider170);
    hlayout_slider_170->addStretch();
    DLabel *lb_170 = new DLabel("170");
    lb_170->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_lbBaud170, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(lb_170, DFontSizeManager::T8, QFont::Normal);

    m_lbBaud310 = new DLabel(this);
    m_lbBaud310->setMaximumWidth(20);
    m_lbBaud310->setMinimumHeight(22);
    m_lbBaud310->setAlignment(Qt::AlignCenter);
    auto hlayout_lb_baud_310  = new QHBoxLayout;
    hlayout_lb_baud_310->addStretch();
    hlayout_lb_baud_310->addWidget(m_lbBaud310);
    hlayout_lb_baud_310->addStretch();
    m_slider310 = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(m_slider310, AC_slider_310);
    AC_SET_ACCESSIBLE_NAME(m_slider310, AC_slider_310);
    m_slider310->setMinimum(-20);
    m_slider310->setMaximum(20);
    m_slider310->setPageStep(1);
    auto hlayout_slider_310  = new QHBoxLayout;
    hlayout_slider_310->addStretch();
    hlayout_slider_310->addWidget(m_slider310);
    hlayout_slider_310->addStretch();
    DLabel *lb_310 = new DLabel("310");
    lb_310->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_lbBaud310, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(lb_310, DFontSizeManager::T8, QFont::Normal);

    m_lbBaud600 = new DLabel(this);
    m_lbBaud600->setMaximumWidth(20);
    m_lbBaud600->setMinimumHeight(22);
    m_lbBaud600->setAlignment(Qt::AlignCenter);
    auto hlayout_lb_baud_600  = new QHBoxLayout;
    hlayout_lb_baud_600->addStretch();
    hlayout_lb_baud_600->addWidget(m_lbBaud600);
    hlayout_lb_baud_600->addStretch();
    m_slider600 = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(m_slider600, AC_slider_600);
    AC_SET_ACCESSIBLE_NAME(m_slider600, AC_slider_600);
    m_slider600->setMinimum(-20);
    m_slider600->setMaximum(20);
    m_slider600->setPageStep(1);
    auto hlayout_slider_600  = new QHBoxLayout;
    hlayout_slider_600->addStretch();
    hlayout_slider_600->addWidget(m_slider600);
    hlayout_slider_600->addStretch();
    DLabel *lb_600 = new DLabel("600");
    lb_600->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_lbBaud600, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(lb_600, DFontSizeManager::T8, QFont::Normal);

    m_lbBaud1K = new DLabel(this);
    m_lbBaud1K->setMaximumWidth(20);
    m_lbBaud1K->setMinimumHeight(22);
    m_lbBaud1K->setAlignment(Qt::AlignCenter);
    auto hlayout_lb_baud_1K  = new QHBoxLayout;
    hlayout_lb_baud_1K->addStretch();
    hlayout_lb_baud_1K->addWidget(m_lbBaud1K);
    hlayout_lb_baud_1K->addStretch();
    m_slider1K = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(m_slider1K, AC_slider_1K);
    AC_SET_ACCESSIBLE_NAME(m_slider1K, AC_slider_1K);
    m_slider1K->setMinimum(-20);
    m_slider1K->setMaximum(20);
    m_slider1K->setPageStep(1);
    auto hlayout_slider_1K  = new QHBoxLayout;
    hlayout_slider_1K->addStretch();
    hlayout_slider_1K->addWidget(m_slider1K);
    hlayout_slider_1K->addStretch();
    DLabel *lb_1K = new DLabel("1K");
    lb_1K->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_lbBaud1K, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(lb_1K, DFontSizeManager::T8, QFont::Normal);

    m_lbBaud3K = new DLabel(this);
    m_lbBaud3K->setMaximumWidth(20);
    m_lbBaud3K->setMinimumHeight(22);
    m_lbBaud3K->setAlignment(Qt::AlignCenter);
    auto hlayout_lb_baud_3K  = new QHBoxLayout;
    hlayout_lb_baud_3K->addStretch();
    hlayout_lb_baud_3K->addWidget(m_lbBaud3K);
    hlayout_lb_baud_3K->addStretch();
    m_slider3K = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(m_slider3K, AC_slider_3K);
    AC_SET_ACCESSIBLE_NAME(m_slider3K, AC_slider_3K);
    m_slider3K->setMinimum(-20);
    m_slider3K->setMaximum(20);
    m_slider3K->setPageStep(1);
    auto hlayout_slider_3K  = new QHBoxLayout;
    hlayout_slider_3K->addStretch();
    hlayout_slider_3K->addWidget(m_slider3K);
    hlayout_slider_3K->addStretch();
    DLabel *lb_3K = new DLabel("3K");
    lb_3K->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_lbBaud3K, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(lb_3K, DFontSizeManager::T8, QFont::Normal);

    m_lbBaud6K = new DLabel(this);
    m_lbBaud6K->setMaximumWidth(20);
    m_lbBaud6K->setMinimumHeight(22);
    m_lbBaud6K->setAlignment(Qt::AlignCenter);
    auto hlayout_lb_baud_6K  = new QHBoxLayout;
    hlayout_lb_baud_6K->addStretch();
    hlayout_lb_baud_6K->addWidget(m_lbBaud6K);
    hlayout_lb_baud_6K->addStretch();
    m_slider6K = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(m_slider6K, AC_slider_6K);
    AC_SET_ACCESSIBLE_NAME(m_slider6K, AC_slider_6K);
    m_slider6K->setMinimum(-20);
    m_slider6K->setMaximum(20);
    m_slider6K->setPageStep(1);
    auto hlayout_slider_6K  = new QHBoxLayout;
    hlayout_slider_6K->addStretch();
    hlayout_slider_6K->addWidget(m_slider6K);
    hlayout_slider_6K->addStretch();
    DLabel *lb_6K = new DLabel("6K");
    lb_6K->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_lbBaud6K, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(lb_6K, DFontSizeManager::T8, QFont::Normal);

    m_lbBaud12K = new DLabel(this);
    m_lbBaud12K->setMaximumWidth(20);
    m_lbBaud12K->setMinimumHeight(22);
    m_lbBaud12K->setAlignment(Qt::AlignCenter);
    auto hlayout_lb_baud_12K  = new QHBoxLayout;
    hlayout_lb_baud_12K->addStretch();
    hlayout_lb_baud_12K->addWidget(m_lbBaud12K);
    hlayout_lb_baud_12K->addStretch();
    m_slider12K = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(m_slider12K, AC_slider_12K);
    AC_SET_ACCESSIBLE_NAME(m_slider12K, AC_slider_12K);
    m_slider12K->setMinimum(-20);
    m_slider12K->setMaximum(20);
    m_slider12K->setPageStep(1);
    auto hlayout_slider_12K  = new QHBoxLayout;
    hlayout_slider_12K->addStretch();
    hlayout_slider_12K->addWidget(m_slider12K);
    hlayout_slider_12K->addStretch();
    DLabel *lb_12K = new DLabel("12K");
    lb_12K->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_lbBaud12K, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(lb_12K, DFontSizeManager::T8, QFont::Normal);

    m_lbBaud14K = new DLabel(this);
    m_lbBaud14K->setMaximumWidth(20);
    m_lbBaud14K->setMinimumHeight(22);
    m_lbBaud14K->setAlignment(Qt::AlignCenter);
    auto hlayout_lb_baud_14K  = new QHBoxLayout;
    hlayout_lb_baud_14K->addStretch();
    hlayout_lb_baud_14K->addWidget(m_lbBaud14K);
    hlayout_lb_baud_14K->addStretch();
    m_slider14K = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(m_slider14K, AC_slider_14K);
    AC_SET_ACCESSIBLE_NAME(m_slider14K, AC_slider_14K);
    m_slider14K->setMinimum(-20);
    m_slider14K->setMaximum(20);
    m_slider14K->setPageStep(1);
    auto hlayout_slider_14K  = new QHBoxLayout;
    hlayout_slider_14K->addStretch();
    hlayout_slider_14K->addWidget(m_slider14K);
    hlayout_slider_14K->addStretch();
    DLabel *lb_14K = new DLabel("14K");
    lb_14K->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_lbBaud14K, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(lb_14K, DFontSizeManager::T8, QFont::Normal);

    m_lbBaud16K = new DLabel(this);
    m_lbBaud16K->setMaximumWidth(20);
    m_lbBaud16K->setMinimumHeight(22);
    m_lbBaud16K->setAlignment(Qt::AlignCenter);
    auto hlayout_lb_baud_16K  = new QHBoxLayout;
    hlayout_lb_baud_16K->addStretch();
    hlayout_lb_baud_16K->addWidget(m_lbBaud16K);
    hlayout_lb_baud_16K->addStretch();
    m_slider16K = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(m_slider16K, AC_slider_16K);
    AC_SET_ACCESSIBLE_NAME(m_slider16K, AC_slider_16K);
    m_slider16K->setMinimum(-20);
    m_slider16K->setMaximum(20);
    m_slider16K->setPageStep(1);
    auto hlayout_slider_16K  = new QHBoxLayout;
    hlayout_slider_16K->addStretch();
    hlayout_slider_16K->addWidget(m_slider16K);
    hlayout_slider_16K->addStretch();
    DLabel *lb_16K = new DLabel("16K");
    lb_16K->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_lbBaud16K, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(lb_16K, DFontSizeManager::T8, QFont::Normal);

    m_btnDefault = new DPushButton(DequalizerDialog::tr("Restore Defaults"));
    m_btnDefault->setMinimumSize(240, 36);
    AC_SET_OBJECT_NAME(m_btnDefault, AC_Restore);
    AC_SET_ACCESSIBLE_NAME(m_btnDefault, AC_Restore);

    auto vlayout0  = new QVBoxLayout;
    vlayout0->setSpacing(0);
    auto vlayout1  = new QVBoxLayout;
    vlayout1->setSpacing(0);
    auto vlayout2  = new QVBoxLayout;
    vlayout2->setSpacing(0);
    auto vlayout3  = new QVBoxLayout;
    vlayout3->setSpacing(0);
    auto vlayout4  = new QVBoxLayout;
    vlayout4->setSpacing(0);
    auto vlayout5  = new QVBoxLayout;
    vlayout5->setSpacing(0);
    auto vlayout6  = new QVBoxLayout;
    vlayout6->setSpacing(0);
    auto vlayout7  = new QVBoxLayout;
    vlayout7->setSpacing(0);
    auto vlayout8  = new QVBoxLayout;
    vlayout8->setSpacing(0);
    auto vlayout9  = new QVBoxLayout;
    vlayout9->setSpacing(0);
    auto vlayout10 = new QVBoxLayout;
    vlayout10->setSpacing(0);

    vlayout0->addLayout(hlayout_lb_baud_pre);
    vlayout0->addLayout(hlayout_slider_pre, 100);
    vlayout0->addWidget(lb_preamplifier);

    vlayout1->addLayout(hlayout_lb_baud_60);
    vlayout1->addLayout(hlayout_slider_60, 100);
    vlayout1->addWidget(lb_60);

    vlayout2->addLayout(hlayout_lb_baud_170);
    vlayout2->addLayout(hlayout_slider_170, 100);
    vlayout2->addWidget(lb_170);

    vlayout3->addLayout(hlayout_lb_baud_310);
    vlayout3->addLayout(hlayout_slider_310, 100);
    vlayout3->addWidget(lb_310);

    vlayout4->addLayout(hlayout_lb_baud_600);
    vlayout4->addLayout(hlayout_slider_600, 100);
    vlayout4->addWidget(lb_600);

    vlayout5->addLayout(hlayout_lb_baud_1K);
    vlayout5->addLayout(hlayout_slider_1K, 100);
    vlayout5->addWidget(lb_1K);

    vlayout6->addLayout(hlayout_lb_baud_3K);
    vlayout6->addLayout(hlayout_slider_3K, 100);
    vlayout6->addWidget(lb_3K);

    vlayout7->addLayout(hlayout_lb_baud_6K);
    vlayout7->addLayout(hlayout_slider_6K, 100);
    vlayout7->addWidget(lb_6K);

    vlayout8->addLayout(hlayout_lb_baud_12K);
    vlayout8->addLayout(hlayout_slider_12K, 100);
    vlayout8->addWidget(lb_12K);

    vlayout9->addLayout(hlayout_lb_baud_14K);
    vlayout9->addLayout(hlayout_slider_14K, 100);
    vlayout9->addWidget(lb_14K);

    vlayout10->addLayout(hlayout_lb_baud_16K);
    vlayout10->addLayout(hlayout_slider_16K, 100);
    vlayout10->addWidget(lb_16K);

    auto hlayout  = new QHBoxLayout;
    hlayout->addLayout(vlayout0);
    hlayout->addLayout(vlayout1);
    hlayout->addLayout(vlayout2);
    hlayout->addLayout(vlayout3);
    hlayout->addLayout(vlayout4);
    hlayout->addLayout(vlayout5);
    hlayout->addLayout(vlayout6);
    hlayout->addLayout(vlayout7);
    hlayout->addLayout(vlayout8);
    hlayout->addLayout(vlayout9);
    hlayout->addLayout(vlayout10);
    hlayout->setSpacing(5);

    slWidget = new DBlurEffectWidget;
    slWidget->setBlurRectXRadius(8);
    slWidget->setBlurRectYRadius(8);
    slWidget->setRadius(8);
    slWidget->setBlurEnabled(true);
    slWidget->setMode(DBlurEffectWidget::GaussianBlur);
    slWidget->setMinimumWidth(517);
    slWidget->setMinimumHeight(264);
    slWidget->setLayout(hlayout);
    slWidget->setContentsMargins(10, 4, 9, 18);
    AC_SET_OBJECT_NAME(slWidget, AC_slWidget);
    AC_SET_ACCESSIBLE_NAME(slWidget, AC_slWidget);

    auto vlay  = new QVBoxLayout;
    //第一行
    vlay->addWidget(m_titleLabel);
    //第二行
    auto hlay  = new QHBoxLayout;
    hlay->addWidget(m_switchLabel, Qt::AlignLeft);
    hlay->addWidget(m_switchBtn, Qt::AlignLeft);
    hlay->addWidget(m_comBox, 20, Qt::AlignCenter);
    hlay->addWidget(m_saveBtn, Qt::AlignRight);

    //第三行
    vlay->addLayout(hlay);
    vlay->addStretch(2);
    vlay->addWidget(slWidget);
    vlay->addWidget(m_btnDefault, 0, Qt::AlignCenter);
    vlay->setContentsMargins(20, 10, 20, 20);

    //均衡器页面
    auto mequalizer = new DWidget(this);
    mequalizer->resize(537, 393);
    mequalizer->setLayout(vlay);
    AC_SET_OBJECT_NAME(mequalizer, AC_mequalizer);
    AC_SET_ACCESSIBLE_NAME(mequalizer, AC_mequalizer);

    auto mtabwidget = new DTabWidget;
    AC_SET_OBJECT_NAME(mtabwidget, AC_mtabwidget);
    AC_SET_ACCESSIBLE_NAME(mtabwidget, AC_mtabwidget);
    mtabwidget->setTabPosition(QTabWidget::West);
    mtabwidget->setAutoFillBackground(true);
    mtabwidget->setDocumentMode(true);
    mtabwidget->resize(720, 463);
    mtabwidget->addTab(mequalizer, DequalizerDialog::tr("Equalizer"));
    m_tabStyle = new CustomTabStyle;
    mtabwidget->tabBar()->setStyle(m_tabStyle);
    mtabwidget->tabBar()->setContentsMargins(10, 0, 10, 0);

    auto mTitlebar = new DTitlebar(this);
    mTitlebar->setTitle("");
    mTitlebar->resize(720, 50);
    AC_SET_OBJECT_NAME(mTitlebar, AC_mTitlebar);
    AC_SET_ACCESSIBLE_NAME(mTitlebar, AC_mTitlebar);

    //垂直布局TabWidget和TitleBar
    auto  mlayout = new QVBoxLayout;
    mlayout->addWidget(mTitlebar);
    mlayout->addWidget(mtabwidget);
    mlayout->setSpacing(0);
    mlayout->setMargin(0);
    this->setLayout(mlayout);

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
    if (slider == m_sliderPre) {
        Q_EMIT this->setEqualizerpre(value);
    } else {
        int bandIndex = -1;
        if (slider == m_slider60) {
            bandIndex = 0;
        } else if (slider == m_slider170) {
            bandIndex = 1;
        } else if (slider == m_slider310) {
            bandIndex = 2;
        } else if (slider == m_slider600) {
            bandIndex = 3;
        } else if (slider == m_slider1K) {
            bandIndex = 4;
        } else if (slider == m_slider3K) {
            bandIndex = 5;
        } else if (slider == m_slider6K) {
            bandIndex = 6;
        } else if (slider == m_slider12K) {
            bandIndex = 7;
        } else if (slider == m_slider14K) {
            bandIndex = 8;
        } else if (slider == m_slider16K) {
            bandIndex = 9;
        }
        if (bandIndex == -1) {
            return;
        }
        Q_EMIT this->setEqualizerbauds(bandIndex, value);
    }
}


DequalizerDialog::DequalizerDialog(QWidget *parent):
    DAbstractDialog(parent)
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
    this->m_saveMessage  = new DFloatingMessage(DFloatingMessage::TransientType, this);
    this->m_saveMessage->setFont(font);
    this->m_saveMessage->setIcon(QIcon::fromTheme("notify_success"));
    this->m_saveMessage->setMessage(tr("Sound Effects Saved"));
    int Minwid = fm.width(tr("Sound Effects Saved")) + 70;
    this->m_saveMessage->setMinimumSize(Minwid, 60);
    this->m_saveMessage->setDuration(2000);
    this->m_saveMessage->move(width() / 2 - 80, height() - 70);
    this->m_saveMessage->hide();
}

DequalizerDialog::~DequalizerDialog()
{
    delete this->m_saveMessage;
    delete this->m_tabStyle;
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
        Q_EMIT setEqualizerIndex(1);
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
            if (slider == this->m_sliderPre) {
                MusicSettings::setOption("equalizer.all." + slider->objectName(), 12);
            } else {
                MusicSettings::setOption("equalizer.all." + slider->objectName(), 0);
            }
        }
    }
}
void DequalizerDialog::selectSlider(QObject *obj, QString SliderVal)
{
    if (obj == this->m_sliderPre) {
        this->m_lbBaudPre->setText(SliderVal);
    } else if (obj == this->m_slider60) {
        this->m_lbBaud60->setText(SliderVal);
    } else if (obj == this->m_slider170) {
        this->m_lbBaud170->setText(SliderVal);
    } else if (obj == this->m_slider310) {
        this->m_lbBaud310->setText(SliderVal);
    } else if (obj == this->m_slider600) {
        this->m_lbBaud600->setText(SliderVal);
    } else if (obj == this->m_slider1K) {
        this->m_lbBaud1K->setText(SliderVal);
    } else if (obj == this->m_slider3K) {
        this->m_lbBaud3K->setText(SliderVal);
    } else if (obj == this->m_slider6K) {
        this->m_lbBaud6K->setText(SliderVal);
    } else if (obj == this->m_slider12K) {
        this->m_lbBaud12K->setText(SliderVal);
    } else if (obj == this->m_slider14K) {
        this->m_lbBaud14K->setText(SliderVal);
    } else if (obj == this->m_slider16K) {
        this->m_lbBaud16K->setText(SliderVal);
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
