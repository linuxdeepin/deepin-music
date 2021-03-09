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

#include "musiclyricwidget.h"
#include "player.h"
#include "global.h"

#include <QDebug>
#include <QFile>
#include <QScrollArea>
#include <QPainter>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QStringListModel>
#include <QAbstractItemDelegate>
#include <QFileInfo>
#include <QPropertyAnimation>
#include <QDir>
#include <QStandardPaths>

#include <DPalette>
#include <DPushButton>

#include "../core/musicsettings.h"

#include "cover.h"
//#include "widget/searchmetalist.h"
//#include "widget/searchmetaitem.h"
#include "lyriclabel.h"
#include "musicsettings.h"
#include <DGuiApplicationHelper>
#include <DBlurEffectWidget>
#include "metadetector.h"

#include "ac-desktop-define.h"
DGUI_USE_NAMESPACE

static const QString defaultLyric = "No Lyric";
static constexpr int AnimationDelay = 400; //ms

MusicLyricWidget::MusicLyricWidget(QWidget *parent)
    : DWidget(parent)
{
    setFocusPolicy(Qt::ClickFocus);
    setAutoFillBackground(true);
    auto palette = this->palette();
    palette.setColor(DPalette::Background, QColor("#F8F8F8"));
    setPalette(palette);

    m_cover = new Cover;
    m_cover->setFixedSize(200, 200);
    m_cover->setObjectName("LyricCover");

    m_leftLayout = new QHBoxLayout();
    m_leftLayout->setContentsMargins(120, 0, 140, 0);
    m_leftLayout->addWidget(m_cover, Qt::AlignLeft | Qt::AlignVCenter);

    m_lyricview = new LyricLabel(false);
    m_nolyric = new DLabel();
    m_nolyric->setAlignment(Qt::AlignCenter);
    m_nolyric->setText(tr("No lyrics yet"));
    QPalette nolyr = m_nolyric->palette();
    QColor nolyrPaletteColor("#C0C6D4");
    nolyrPaletteColor.setAlphaF(0.4);
    nolyr.setColor(QPalette::WindowText, nolyrPaletteColor);
    m_nolyric->setPalette(nolyr);
    m_nolyric->setForegroundRole(QPalette::WindowText);


    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 150);
    layout->addLayout(m_leftLayout, 0);
    layout->addWidget(m_lyricview, 10);
    layout->addWidget(m_nolyric, 10);
    // 根据是否有歌曲播放判断是否显示“无歌词”
    if (Player::getInstance()->getActiveMeta().hash.isEmpty()) {
        m_nolyric->show();
        m_lyricview->hide();
    } else {
        m_nolyric->hide();
        m_lyricview->show();
    }


    AC_SET_OBJECT_NAME(m_lyricview, AC_lyricview);
    AC_SET_ACCESSIBLE_NAME(m_lyricview, AC_lyricview);

    m_backgroundW = new DBlurEffectWidget(this);
//    d->backgroundW->setBlurEnabled(true);
//    d->backgroundW->setMode(DBlurEffectWidget::GaussianBlur);
    m_backgroundW->setLayout(layout);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_backgroundW);
    this->setLayout(mainLayout);

    connect(Player::getInstance(), &Player::signalMediaMetaChanged,
            this, &MusicLyricWidget::onCoverChanged);

    connect(Player::getInstance(), &Player::signalMediaMetaChanged,
            this, &MusicLyricWidget::onMusicPlayed);

    connect(Player::getInstance(), &Player::positionChanged,
            this, &MusicLyricWidget::onProgressChanged);


    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &MusicLyricWidget::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());
}

MusicLyricWidget::~MusicLyricWidget()
{

}

void MusicLyricWidget::updateUI()
{
    MediaMeta meta = Player::getInstance()->getActiveMeta();
    QFileInfo coverInfo(Global::cacheDir() + "/images/" + meta.hash + ".jpg");
    QPixmap cover;
    if (coverInfo.exists()) {
        // 不使用缩略图,使用原图,更加清晰
        cover = MetaDetector::getCoverDataPixmap(meta);
    } else {
        cover = QIcon::fromTheme("cover_max").pixmap(QSize(200, 200));
    }
    m_cover->setCoverPixmap(cover);
    m_cover->update();

    //cut image
    double windowScale = (width() * 1.0) / height();
    int imageWidth = static_cast<int>(cover.height() * windowScale);
    QImage coverImage;
    if (imageWidth > cover.width()) {
        int imageheight = static_cast<int>(cover.width() / windowScale);
        coverImage = cover.toImage().copy(0, (cover.height() - imageheight) / 2, cover.width(), imageheight);
    } else {
        int imageheight = cover.height();
        coverImage = cover.toImage().copy((cover.width() - imageWidth) / 2, 0, imageWidth, imageheight);
    }

    m_backgroundW->setSourceImage(coverImage);
    m_backgroundW->update();
}

void MusicLyricWidget::showAnimation()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

    animation->setDuration(AnimationDelay);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, height()));
    animation->setEndValue(QPoint(0, 50));

    connect(animation, &QPropertyAnimation::finished,
            animation, &QPropertyAnimation::deleteLater);

    this->lower();
    this->show();

    animation->start();
}

void MusicLyricWidget::closeAnimation()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

    animation->setDuration(AnimationDelay);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, 50));
    animation->setEndValue(QPoint(0, this->height()));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    animation->connect(animation, &QPropertyAnimation::finished,
                       this, &MusicLyricWidget::signalAutoHidden);
    animation->connect(animation, &QPropertyAnimation::finished,
                       this, &MusicLyricWidget::hide);

    animation->start();
}

void MusicLyricWidget::resizeEvent(QResizeEvent *event)
{
    updateUI();
    QWidget::resizeEvent(event);
}

void MusicLyricWidget::onProgressChanged(qint64 value, qint64 /*length*/)
{
    m_lyricview->postionChanged(value);
}

void MusicLyricWidget::onMusicPlayed(MediaMeta meta)
{
    Q_UNUSED(meta)
    QFileInfo fileInfo(Player::getInstance()->getActiveMeta().localPath);
    QString lrcPath = fileInfo.dir().path() + QDir::separator() + fileInfo.completeBaseName() + ".lrc";
    QFile file(lrcPath);
    if (!file.exists()) {
        m_nolyric->show();
        m_lyricview->hide();
    } else {
        m_nolyric->hide();
        m_lyricview->show();
    }
    m_lyricview->getFromFile(lrcPath);
}

void MusicLyricWidget::onCoverChanged(MediaMeta meta)
{
    Q_UNUSED(meta)
    this->updateUI();
}

void MusicLyricWidget::setThemeType(int type)
{
    if (type == 1) {
        auto palette = this->palette();
        palette.setColor(DPalette::Background, QColor("#F8F8F8"));
        setPalette(palette);

        QColor backMaskColor(255, 255, 255, 140);
        m_backgroundW->setMaskColor(backMaskColor);

        QPalette nolyr = m_nolyric->palette();
        QColor nolyrPaletteColor("#000000");
        nolyrPaletteColor.setAlphaF(0.3);
        nolyr.setColor(QPalette::WindowText, nolyrPaletteColor);
        m_nolyric->setPalette(nolyr);
        m_nolyric->setForegroundRole(QPalette::WindowText);
    } else {
        auto palette = this->palette();
        palette.setColor(DPalette::Background, QColor("#252525"));
        setPalette(palette);

        QColor backMaskColor(37, 37, 37, 140);
        m_backgroundW->setMaskColor(backMaskColor);

        QPalette nolyr = m_nolyric->palette();
        QColor nolyrPaletteColor("#C0C6D4");
        nolyrPaletteColor.setAlphaF(0.4);
        nolyr.setColor(QPalette::WindowText, nolyrPaletteColor);
        m_nolyric->setPalette(nolyr);
        m_nolyric->setForegroundRole(QPalette::WindowText);
    }
}

