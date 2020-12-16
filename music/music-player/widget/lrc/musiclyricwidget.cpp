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
#include "widget/musicimagebutton.h"
#include "musicsettings.h"
#include <DGuiApplicationHelper>
#include <DBlurEffectWidget>

#include "ac-desktop-define.h"
DGUI_USE_NAMESPACE

//static const int LyricLineHeight = 40;
static const QString defaultLyric = "No Lyric";
static constexpr int AnimationDelay = 400; //ms

MusicLyricWidget::MusicLyricWidget(QWidget *parent)
    : DWidget(parent)
{
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
    nolyr.setColor(QPalette::WindowText, QColor(85, 85, 85, 102));
    m_nolyric->setPalette(nolyr);

    m_serachbt = new MusicImageButton(":/mpimage/light/normal/search_normal.svg",
                                      ":/mpimage/light/normal/search_normal.svg",
                                      ":/mpimage/light/normal/search_normal.svg");
    m_serachbt->setProperty("typeName", true);
    m_serachbt->setPropertyPic("typeName", false, ":/mpimage/light/normal/back_normal.svg",
                               ":/mpimage/light/normal/back_normal.svg",
                               ":/mpimage/light/normal/back_normal.svg");
    m_serachbt->setFixedSize(48, 48);
    m_serachbt->hide();


    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 150);
    layout->addLayout(m_leftLayout, 0);
    layout->addWidget(m_lyricview, 10);
    layout->addWidget(m_nolyric, 10);
    m_nolyric->hide();

    m_backgroundW = new DBlurEffectWidget(this);
//    d->backgroundW->setBlurEnabled(true);
//    d->backgroundW->setMode(DBlurEffectWidget::GaussianBlur);
    m_backgroundW->setLayout(layout);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_backgroundW);
    this->setLayout(mainLayout);

    int themeType = DGuiApplicationHelper::instance()->themeType();
    slotTheme(themeType);

    connect(m_serachbt, &DPushButton::clicked, this, &MusicLyricWidget::onsearchBt);

    connect(Player::getInstance(), &Player::signalMediaMetaChanged,
            this, &MusicLyricWidget::onCoverChanged);

    connect(Player::getInstance(), &Player::signalMediaMetaChanged,
            this, &MusicLyricWidget::onMusicPlayed);

    connect(Player::getInstance(), &Player::positionChanged,
            this, &MusicLyricWidget::onProgressChanged);
}

MusicLyricWidget::~MusicLyricWidget()
{

}

void MusicLyricWidget::updateUI()
{
    MediaMeta meta = Player::getInstance()->getActiveMeta();
    QFileInfo coverInfo(Global::cacheDir() + "/images/" + meta.hash + ".jpg");
    QImage cover;
    if (coverInfo.exists()) {
        cover = QImage(Global::cacheDir() + "/images/" + meta.hash + ".jpg");
    } else {
        cover = m_defaultImage;
    }
    m_cover->setCoverPixmap(QPixmap::fromImage(cover));
    m_cover->update();

    //cut image
    double windowScale = (width() * 1.0) / height();
    int imageWidth = static_cast<int>(cover.height() * windowScale);
    QImage coverImage;
    if (imageWidth > cover.width()) {
        int imageheight = static_cast<int>(cover.width() / windowScale);
        coverImage = cover.copy(0, (cover.height() - imageheight) / 2, cover.width(), imageheight);
    } else {
        int imageheight = cover.height();
        coverImage = cover.copy((cover.width() - imageWidth) / 2, 0, imageWidth, imageheight);
    }

    m_backgroundW->setSourceImage(coverImage);
    m_backgroundW->update();
}

void MusicLyricWidget::showAnimation(const QSize &size)
{
    this->resize(size);

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

void MusicLyricWidget::closeAnimation(const QSize &size)
{
    this->resize(size);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

    animation->setDuration(AnimationDelay);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, 50));
    animation->setEndValue(QPoint(0, size.height()));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    animation->connect(animation, &QPropertyAnimation::finished,
                       this, &MusicLyricWidget::hide);

    animation->start();
}

QString MusicLyricWidget::defaultCover() const
{
    return m_defaultCover;
}

void MusicLyricWidget::checkHiddenSearch(QPoint mousePos)
{
    Q_UNUSED(mousePos)

}

void MusicLyricWidget::resizeEvent(QResizeEvent *event)
{
    updateUI();
    QWidget::resizeEvent(event);
}

void MusicLyricWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_serachflag) {
        onsearchBt();
    }
    QWidget::mousePressEvent(event);
}

void MusicLyricWidget::onProgressChanged(qint64 value, qint64 /*length*/)
{
    m_lyricview->postionChanged(value);
}

void MusicLyricWidget::onMusicPlayed()
{
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

void MusicLyricWidget::onCoverChanged()
{
    this->updateUI();
}

void MusicLyricWidget::setDefaultCover(QString defaultCover)
{
    m_defaultCover = defaultCover;
}

void MusicLyricWidget::onsearchBt()
{
    m_serachflag = !m_serachflag;
    if (m_serachflag) {
        m_serachbt->setProperty("typeName", false);
        m_cover->hide();
        m_leftLayout->setContentsMargins(51, 21, 51, 19);
    } else {
        m_serachbt->setProperty("typeName", true);
        m_cover->show();
        m_leftLayout->setContentsMargins(120, 190, 140, 160);
    }
    m_serachbt->update();
}

void MusicLyricWidget::slotonsearchresult(QString path)
{
    m_lyricview->getFromFile(path);
}

void MusicLyricWidget::slotTheme(int type)
{
    if (type == 0)
        type = DGuiApplicationHelper::instance()->themeType();
    QString rStr;
    if (type == 1) {
        rStr = "light";
    } else {
        rStr = "dark";
    }
    if (type == 1) {
        auto palette = this->palette();
        palette.setColor(DPalette::Background, QColor("#F8F8F8"));
        setPalette(palette);

        QColor backMaskColor(255, 255, 255, 140);
        m_backgroundW->setMaskColor(backMaskColor);
    } else {
        auto palette = this->palette();
        palette.setColor(DPalette::Background, QColor("#252525"));
        setPalette(palette);

        QColor backMaskColor(37, 37, 37, 140);
        m_backgroundW->setMaskColor(backMaskColor);
    }
    m_serachbt->setPropertyPic(QString(":/mpimage/%1/normal/search_normal.svg").arg(rStr),
                               QString(":/mpimage/%1/normal/search_normal.svg").arg(rStr),
                               QString(":/mpimage/%1/normal/search_normal.svg").arg(rStr));

    m_serachbt->setPropertyPic("typeName", false, QString(":/mpimage/%1/normal/back_normal.svg").arg(rStr),
                               QString(":/mpimage/%1/normal/back_normal.svg").arg(rStr),
                               QString(":/mpimage/%1/normal/back_normal.svg").arg(rStr));

    m_lyricview->slotTheme(type);
}

void MusicLyricWidget::onContextSearchFinished(const QString &context, const QList<DMusic::SearchMeta> &metalist)
{
    //Q_D(MUsicLyricWidget);

    //TODO: check context
    Q_UNUSED(context);
    Q_UNUSED(metalist)
}


