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

#include "soundvolume.h"

#include <QDebug>
#include <QPainter>
#include <QWheelEvent>
#include <DGuiApplicationHelper>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QIcon>
#include <QPainterPath>

#include <DUtil>
#include <DHiDPIHelper>

#include "player.h"
#include "musicsettings.h"
#include "ac-desktop-define.h"

SoundVolume::SoundVolume(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("SoundVolume");
    setProperty("_dm_keep_on_click", true);

    setFixedSize(62, 201);
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 16, 0, 14);
    layout->setSpacing(0);


    m_volPersent = new DLabel(this);
    auto titleFont = m_volPersent->font();
    titleFont.setFamily("SourceHanSansSC");
    titleFont.setWeight(QFont::Medium);

// 解决字体不会根据系统字体大小改变问题
//    titleFont.setPixelSize(14);
    m_volPersent->setFont(titleFont);
    m_volPersent->setText("100%");

    m_btSound = new DToolButton();
    m_btSound->setIcon(QIcon::fromTheme("volume_low"));
    //d->sound->setShortcut(QKeySequence(QLatin1String("M")));
    m_btSound->setFixedSize(30, 30);
    m_btSound->setIconSize(QSize(30, 30));

    m_volSlider = new DSlider(Qt::Vertical);
    m_volSlider->setMinimum(0);
    m_volSlider->setMaximum(100);
    m_volSlider->slider()->setSingleStep(Player::VolumeStep);
    m_volSlider->slider()->setFixedHeight(120);
    m_volSlider->setFixedWidth(24);
    m_volSlider->setIconSize(QSize(15, 15));
    m_volSlider->setMouseWheelEnabled(true);
    m_volSlider->setValue(Player::getInstance()->getVolume());
    m_volPersent->setText(QString::number(Player::getInstance()->getVolume()) + QString("%"));
    connect(m_volSlider, &DSlider::valueChanged, this, &SoundVolume::slotSetVolume);

    AC_SET_OBJECT_NAME(m_volSlider, AC_DSlider);
    AC_SET_ACCESSIBLE_NAME(m_volSlider, AC_DSlider);

    layout->addStretch();
    layout->addWidget(m_volPersent, 0, Qt::AlignTop | Qt::AlignHCenter);
    layout->addWidget(m_volSlider, 0, Qt::AlignCenter);
    layout->addWidget(m_btSound, 0, Qt::AlignHCenter);
    layout->addStretch();
    setFixedSize(62, 201);

    auto *bodyShadow = new QGraphicsDropShadowEffect;
    bodyShadow->setBlurRadius(10.0);
    bodyShadow->setColor(QColor(0, 0, 0,  255 / 10));
    bodyShadow->setOffset(0, 2.0);
    this->setGraphicsEffect(bodyShadow);

    m_timer = new QTimer(this);
    m_timer->setInterval(3000);
    connect(m_timer, &QTimer::timeout, this, &SoundVolume::slotTimeOut);

    connect(m_btSound, &DToolButton::pressed, this, &SoundVolume::slotSoundClick);
    connect(Player::getInstance(), &Player::signalMutedChanged, this, &SoundVolume::flushVolumeIcon);


    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &SoundVolume::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());
}

SoundVolume::~SoundVolume()
{

}

int SoundVolume::volume() const
{
    return m_volSlider->value();
}

void SoundVolume::setVolume(int value)
{
    m_volSlider->setValue(value);
}

void SoundVolume::flushVolumeIcon()
{
    int volume = Player::getInstance()->getVolume();

    if (Player::getInstance()->getMuted() || volume == 0) {
        m_btSound->setIcon(QIcon::fromTheme("mute"));
    } else {
        if (volume > 77) {
            m_btSound->setIcon(QIcon::fromTheme("volume"));
        } else if (volume > 33) {
            m_btSound->setIcon(QIcon::fromTheme("volume_mid"));
        } else {
            m_btSound->setIcon(QIcon::fromTheme("volume_low"));
        }
    }

    emit sigVolumeChanged();
}

void SoundVolume::setMouseIn(bool in)
{
    m_mouseIn = in;
}
// 外部调用，决定是否启动隐藏定时器
void SoundVolume::startTimer(bool start)
{
    if (start) {
        m_timer->start();
    } else {
        m_timer->stop();
    }
}

void SoundVolume::delayHide()
{
    m_mouseIn = false;
    DUtil::TimerSingleShot(1000, [this]() {
        if (!m_mouseIn) {
            hide();
            Q_EMIT delayAutoHide();
        }
    });
}

void SoundVolume::slotSetVolume(int volume)
{
    updateUI(volume);
}

void SoundVolume::updateUI(int volume)
{
    Player::getInstance()->setVolume(volume);

    flushVolumeIcon();
    m_volPersent->setText(QString::number(volume) + QString("%"));
}

void SoundVolume::initBgImage()
{
    QPainter pai;

    const qreal radius = 20;
    const qreal triHeight = 30;
    const qreal height = this->height() - triHeight;
    const qreal width = this->width();

    bgImage = QPixmap(this->size());
    bgImage.fill(QColor(0, 0, 0, 0));
    pai.begin(&bgImage);
    pai.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);

    // 背景上矩形，半边
    QPainterPath pathRect;
    pathRect.moveTo(radius, 0);
    pathRect.lineTo(width / 2, 0);
    pathRect.lineTo(width / 2, height);
    pathRect.lineTo(0, height);
    pathRect.lineTo(0, radius);
    pathRect.arcTo(QRectF(QPointF(0, 0), QPointF(2 * radius, 2 * radius)), 180.0, -90.0);

    // 背景下三角，半边
    qreal radius1 = radius / 2;
    QPainterPath pathTriangle;
    pathTriangle.moveTo(0, height - radius1);
    pathTriangle.arcTo(QRectF(QPointF(0, height - radius1), QSizeF(2 * radius1, 2 * radius1)), 180, 60);
    pathTriangle.lineTo(width / 2, this->height());
    qreal radius2 = radius / 4;
    pathTriangle.arcTo(QRectF(QPointF(width / 2 - radius2, this->height() - radius2 * 2 - 2), QSizeF(2 * radius2, 2 * radius2)), 220, 100);
    pathTriangle.lineTo(width / 2, height);


    // 背景颜色
    QColor bgColor;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        bgColor = QColor(43, 43, 43);
    } else {
        bgColor = QColor(255, 255, 255);
    }

    // 正向绘制
    pai.fillPath(pathRect, bgColor);
    pai.fillPath(pathTriangle, bgColor);

    // 坐标系X反转
    pai.translate(width, 0);
    pai.scale(-1, 1);

    // 反向绘制
    pai.fillPath(pathRect, bgColor);
    pai.fillPath(pathTriangle, bgColor);

    pai.end();
}

void SoundVolume::showEvent(QShowEvent *event)
{
    flushVolumeIcon();
    QWidget::showEvent(event);
}

void SoundVolume::enterEvent(QEvent *event)
{
    m_mouseIn = true;
    // 鼠标进入，停止定时器，防止音量条隐藏
    m_timer->stop();
    QWidget::enterEvent(event);
}

void SoundVolume::leaveEvent(QEvent *event)
{
    m_mouseIn = false;
    delayHide();
    QWidget::leaveEvent(event);
}

void SoundVolume::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) {
        m_volSlider->setValue(m_volSlider->value() + 5);
    } else {
        m_volSlider->setValue(m_volSlider->value() - 5);
    }

    QWidget::wheelEvent(event);
}

void SoundVolume::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter pai(this);
    // 按设计修改透明度
    pai.setOpacity(0.95);
    pai.drawPixmap(0, 0, bgImage);
}

void SoundVolume::setThemeType(int type)
{
    Q_UNUSED(type)

    initBgImage();
}

void SoundVolume::slotSoundClick()
{
    bool mute = Player::getInstance()->getMuted();
    Player::getInstance()->setMuted(!mute);
    flushVolumeIcon();
}

void SoundVolume::slotTimeOut()
{
    hide();
    Q_EMIT delayAutoHide();
}
