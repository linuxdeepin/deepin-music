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
#include <DLabel>

#include <DUtil>
#include <DSlider>
#include <DHiDPIHelper>

#include "core/player.h"
#include "widget/soundpixmapbutton.h"
#include "musicsettings.h"

using namespace Dtk::Widget;

class SoundVolumePrivate
{
public:
    SoundVolumePrivate(SoundVolume *parent) : q_ptr(parent) {}

    SoundVolume *q_ptr;
    DLabel      *lblPersent = nullptr;
    DSlider     *volSlider           = nullptr;
    SoundPixmapButton *sound         = nullptr;
    QBrush      background;
    QColor      borderColor = QColor(0, 0, 0,  255 * 2 / 10);

    int         radius      = 20;
    bool        mouseIn     = false;
    int         sThemeType  = 0;
    bool        volMute     = false;

    Q_DECLARE_PUBLIC(SoundVolume)
};

SoundVolume::SoundVolume(QWidget *parent) : QWidget(parent), d_ptr(new SoundVolumePrivate(this))
{
    Q_D(SoundVolume);
    setObjectName("SoundVolume");
    setProperty("_dm_keep_on_click", true);

    setFixedSize(62, 201);
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 16, 0, 14);
    layout->setSpacing(0);

    d->lblPersent = new DLabel(this);
    auto titleFont = d->lblPersent->font();
    titleFont.setFamily("SourceHanSansSC");
    titleFont.setWeight(QFont::Medium);
    titleFont.setPixelSize(14);
    d->lblPersent->setFont(titleFont);
    d->lblPersent->setText("100%");

    d->volSlider = new DSlider(Qt::Vertical);
    d->volSlider->setMinimum(0);
    d->volSlider->setMaximum(100);
    d->volSlider->slider()->setSingleStep(Player::VolumeStep);
    d->volSlider->setValue(50);
    d->volSlider->slider()->setFixedHeight(120);
    d->volSlider->setFixedWidth(24);
    d->volSlider->setIconSize(QSize(15, 15));
    d->volSlider->setMouseWheelEnabled(true);

    d->sound = new SoundPixmapButton();
    //d->sound->setShortcut(QKeySequence(QLatin1String("M")));
    d->sound->setFixedSize(30, 30);
    d->sound->setIconSize(QSize(30, 30));

    d->volMute = MusicSettings::value("base.play.mute").toBool();
    volumeIcon();

    layout->addStretch();
    layout->addWidget(d->lblPersent, 0, Qt::AlignTop | Qt::AlignHCenter);
    layout->addWidget(d->volSlider, 0, Qt::AlignCenter);
    layout->addWidget(d->sound, 0, Qt::AlignHCenter);
    layout->addStretch();
    setFixedSize(62, 201);

    auto *bodyShadow = new QGraphicsDropShadowEffect;
    bodyShadow->setBlurRadius(10.0);
    bodyShadow->setColor(QColor(0, 0, 0,  255 / 10));
    bodyShadow->setOffset(0, 2.0);
    this->setGraphicsEffect(bodyShadow);

//    connect(d->volSlider, &DSlider::valueChanged,
//            this, &SoundVolume::volumeChanged);

    connect(d->volSlider, &DSlider::valueChanged,
    this, [ = ](int volume) {
        if (volume == 0) {
            d->volMute = true;
        } else {
            d->volMute = false;
        }
        volumeIcon();

        Q_EMIT volumeChanged(volume);
    });

    connect(d->sound, &SoundPixmapButton::pressed,
    this, [ = ]() {
        d->volMute = !d->volMute;
        volumeIcon();

        Q_EMIT volumeMute();
    });
}

SoundVolume::~SoundVolume()
{

}

int SoundVolume::volume() const
{
    Q_D(const SoundVolume);
    return d->volSlider->value();
}

QBrush SoundVolume::background() const
{
    Q_D(const SoundVolume);
    return d->background;
}

int SoundVolume::radius() const
{
    Q_D(const SoundVolume);
    return d->radius;
}

QColor SoundVolume::borderColor() const
{
    Q_D(const SoundVolume);
    return d->borderColor;
}

void SoundVolume::setBackground(QBrush background)
{
    Q_D(SoundVolume);
    d->background = background;
}

void SoundVolume::setRadius(int radius)
{
    Q_D(SoundVolume);
    d->radius = radius;
}

void SoundVolume::setBorderColor(QColor borderColor)
{
    Q_D(SoundVolume);
    d->borderColor = borderColor;
}

void SoundVolume::volumeIcon()
{
    Q_D(SoundVolume);
    if (d->sThemeType != 2) {
        if (!d->volMute) {
            d->sound->setIcon(QPixmap(":/mpimage/light/normal/volume_low_normal.svg"));
        } else {
            d->sound->setIcon(QPixmap(":/mpimage/light/normal/mute_normal.svg"));
        }
    } else {
        if (!d->volMute) {
            d->sound->setIcon(QPixmap(":/mpimage/light/checked/volume_low_checked.svg"));
        } else {
            d->sound->setIcon(QPixmap(":/mpimage/light/checked/mute_checked.svg"));
        }
    }
}

void SoundVolume::syncMute(bool mute)
{
    Q_D(SoundVolume);
    d->volMute = mute;
    volumeIcon();

    Q_UNUSED(mute)
}

void SoundVolume::deleyHide()
{
    Q_D(SoundVolume);
    d->mouseIn = false;
    DUtil::TimerSingleShot(1000, [this]() {
        Q_D(SoundVolume);
        if (!d->mouseIn) {
            hide();
        }
    });
}

void SoundVolume::onVolumeChanged(int volume)
{
    Q_D(SoundVolume);
    d->lblPersent->setText(QString::number(volume) + QString("%"));
    d->volSlider->blockSignals(true);
    d->volSlider->setValue(volume);
    d->volSlider->blockSignals(false);
}

void SoundVolume::showEvent(QShowEvent *event)
{
    Q_D(SoundVolume);
    d->mouseIn = true;
    QWidget::showEvent(event);
}

void SoundVolume::enterEvent(QEvent *event)
{
    Q_D(SoundVolume);
    d->mouseIn = true;
    QWidget::enterEvent(event);
}

void SoundVolume::leaveEvent(QEvent *event)
{
    Q_D(SoundVolume);
    d->mouseIn = false;
    deleyHide();
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
    Q_D(const SoundVolume);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    QPainterPath path;

    auto palette = this->palette();

    auto penWidthf = 1.0;
    auto background =  palette.background();
    auto borderColor = d->borderColor;

    const qreal radius = d->radius;;
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

#if 0
    path.moveTo(radius, 0.0);
    path.lineTo(width - radius, 0.0);

    path.arcTo(topLeftRect, 90.0, 90.0);
    path.lineTo(width, height - radius);
    path.lineTo(width / 2, height + triHeight);
    path.lineTo(0.0, height - radius);
    path.lineTo(0.0, radius);

    path.arcTo(topRightRect, 180.0, -90.0);
    path.lineTo(radius, 0.0);
#else
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
#endif

    /*
    FIXME: light: white
    painter.fillPath(path, QColor(49, 49, 49));
    FIXME: light: QColor(0, 0, 0, 51)
    QPen pen(QColor(0, 0, 0, 0.1 * 255));
    */

    if (d->sThemeType == 2) {
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
    Q_D(SoundVolume);
    d->sThemeType = type;

    volumeIcon();
}
