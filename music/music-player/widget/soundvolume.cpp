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

#include "soundvolume.h"

#include <QDebug>
#include <QPainter>
#include <QWheelEvent>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QIcon>

#include <DUtil>
#include <DHiDPIHelper>

#include "player.h"
#include "widget/soundpixmapbutton.h"
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
    titleFont.setPixelSize(14);
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
    connect(m_volSlider, &DSlider::valueChanged, this, &SoundVolume::slotSetVolume);
    m_volSlider->setValue(Player::instance()->getVolume());

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

    connect(m_btSound, &DToolButton::pressed, this, &SoundVolume::slotSoundClick);
    connect(Player::instance(), &Player::mutedChanged, this, &SoundVolume::flushVolumeIcon);
}

SoundVolume::~SoundVolume()
{

}

int SoundVolume::volume() const
{
    return m_volSlider->value();
}

QBrush SoundVolume::background() const
{
    return m_background;
}

void SoundVolume::setVolume(int value)
{
    m_volSlider->setValue(value);
}

int SoundVolume::radius() const
{
    return m_radius;
}

QColor SoundVolume::borderColor() const
{
    return m_borderColor;
}

void SoundVolume::setBackground(QBrush background)
{
    m_background = background;
}

void SoundVolume::setRadius(int radius)
{
    m_radius = radius;
}

void SoundVolume::setBorderColor(QColor borderColor)
{
    m_borderColor = borderColor;
}

void SoundVolume::flushVolumeIcon()
{
    int volume = Player::instance()->getVolume();

    if (Player::instance()->getMuted() || volume == 0) {
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
    bool muteToSet = false;
    if (volume == 0) {
        muteToSet = true;
    } else {
        muteToSet = false;
    }

    // 设置是否静音并刷新按钮图标
    if (Player::instance()->getMuted()) {
        Player::instance()->setMuted(muteToSet);
    }

    Player::instance()->setVolume(volume);

    flushVolumeIcon();
    m_volPersent->setText(QString::number(volume) + QString("%"));
}

void SoundVolume::showEvent(QShowEvent *event)
{
    m_mouseIn = true;
    flushVolumeIcon();
    QWidget::showEvent(event);
}

void SoundVolume::enterEvent(QEvent *event)
{
    m_mouseIn = true;
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
    QWidget::wheelEvent(event);

    /*
    Q_D(SoundVolume);
    if (event->angleDelta().y() > 0) {
        d->volSlider->setValue(d->volSlider->value() + 5);
    } else {
        d->volSlider->setValue(d->volSlider->value() - 5);
    }
    */

}

void SoundVolume::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    QPainterPath path;

    auto palette = this->palette();

    auto penWidthf = 1.0;
    auto background =  palette.background();
    auto borderColor = m_borderColor;

    const qreal radius = m_radius;;
    const qreal triHeight = 12;
    const qreal triWidth = 16;
    const qreal height = this->height() - triHeight;
    const qreal width = this->width();

    QRectF topRightRect(QPointF(0, 0),
                        QPointF(2 * radius, 2 * radius));
    QRectF bottomRightRect(QPointF(0, height - 2 * radius),
                           QPointF(2 * radius, height));
    QRectF topLeftRect(QPointF(width, 0),
                       QPointF(width - 2 * radius, 2 * radius));
    QRectF bottomLeftRect(QPointF(width, height),
                          QPointF(width - 30, height - 30));

    path.moveTo(radius, 0.0);
    path.lineTo(width - radius, 0.0);
    path.arcTo(topLeftRect, 90.0, 90.0);
    path.lineTo(width, height - radius);
    path.arcTo(bottomLeftRect, 180.0, -60.0);

    path.lineTo(width / 2 + 3, height + triHeight);
    path.lineTo(10, height - 2);

    path.arcTo(bottomRightRect, 270.0, -90.0);
    path.lineTo(0.0,  radius);

    path.arcTo(topRightRect, 180.0, -90.0);
    path.lineTo(radius, 0.0);

    /*
    FIXME: light: white
    painter.fillPath(path, QColor(49, 49, 49));
    FIXME: light: QColor(0, 0, 0, 51)
    QPen pen(QColor(0, 0, 0, 0.1 * 255));
    */

    if (m_sThemeType == 2) {
        painter.fillPath(path, QColor(43, 43, 43));

    } else {
        painter.fillPath(path, background);
    }

    QPen pen(borderColor);
    pen.setWidth(penWidthf);
    //painter.strokePath(path, pen);
}
void SoundVolume::slotTheme(int type)
{
    m_sThemeType = type;
}

void SoundVolume::slotSoundClick()
{
    bool mute = Player::instance()->getMuted();
    Player::instance()->setMuted(!mute);
    flushVolumeIcon();
}
