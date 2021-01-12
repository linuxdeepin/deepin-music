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

#include "subsonglistwidget.h"

#include <QDebug>
#include <QFileInfo>
#include <DPalette>

#include <DFrame>
#include <DLabel>
#include <QBitmap>
#include <DImageButton>
#include <DPushButton>
#include <DApplicationHelper>
#include <DHiDPIHelper>

#include "global.h"
#include "commonservice.h"
#include "dialogList/musiclistinfoview.h"
#include "ac-desktop-define.h"
#include "player.h"
#include "musiclistdialog.h"
DWIDGET_USE_NAMESPACE

void SubSonglistWidget::initUI()
{
    setAutoFillBackground(true);
    auto palette = this->palette();
    QColor BackgroundColor("#F8F8F8");
    palette.setColor(DPalette::Window, BackgroundColor);
    setPalette(palette);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 50, 0, 0);

    m_titleImage = new QLabel;
    m_titleImage->setForegroundRole(DPalette::BrightText);
    m_titleImage->setFixedHeight(150);
    QPixmap m_bgImage = DHiDPIHelper::loadNxPixmap(":/common/image/cover_max.svg");
    setTitleImage(m_bgImage);
    mainLayout->addWidget(m_titleImage, 0);

    QVBoxLayout *titleLayout = new QVBoxLayout(m_titleImage);
    titleLayout->setSpacing(5);
    titleLayout->setContentsMargins(28, 15, 0, 20);

    m_titleLabel = new DLabel();
    m_titleLabel->setForegroundRole(DPalette::BrightText);
    titleLayout->addWidget(m_titleLabel, 1);
    m_infoLabel = new DLabel();
    m_infoLabel->setForegroundRole(DPalette::BrightText);
    titleLayout->addWidget(m_infoLabel, 1);

    QHBoxLayout *btLayout = new QHBoxLayout(m_titleImage);
    btLayout->setSpacing(5);
    btLayout->setContentsMargins(0, 16, 0, 0);
    titleLayout->addLayout(btLayout, 1);

    m_btPlayAll = new DPushButton;
    auto btPlayAllFont = m_btPlayAll->font();
    btPlayAllFont.setPixelSize(14);
    m_btPlayAll->setFont(btPlayAllFont);
    auto playAllPalette = m_btPlayAll->palette();
    playAllPalette.setColor(DPalette::ButtonText, Qt::white);
    playAllPalette.setColor(DPalette::Dark, QColor(Qt::red));
    playAllPalette.setColor(DPalette::Light, QColor(Qt::red));
    m_btPlayAll->setPalette(playAllPalette);
    m_btPlayAll->setIcon(QIcon(":/mpimage/light/normal/play_all_normal.svg"));
    m_btPlayAll->setText(MusicListDialog::tr("Play All"));
    m_btPlayAll->setFocusPolicy(Qt::NoFocus);
    m_btPlayAll->setIconSize(QSize(18, 18));
    m_btPlayAll->setFixedSize(QSize(100, 30));
    btLayout->addWidget(m_btPlayAll, 1);

    m_btRandomPlay = new DPushButton;
    m_btRandomPlay->setFont(btPlayAllFont);
    auto randomPlayPalette = m_btRandomPlay->palette();
    randomPlayPalette.setColor(DPalette::ButtonText, Qt::white);
    randomPlayPalette.setColor(DPalette::Dark, QColor(Qt::darkGray));
    randomPlayPalette.setColor(DPalette::Light, QColor(Qt::darkGray));
    m_btRandomPlay->setPalette(randomPlayPalette);
    m_btRandomPlay->setIcon(QIcon(":/mpimage/light/normal/random_play_normal.svg"));
    m_btRandomPlay->setText(MusicListDialog::tr("Shuffle"));
    m_btRandomPlay->setFocusPolicy(Qt::NoFocus);
    m_btRandomPlay->setIconSize(QSize(18, 18));
    m_btRandomPlay->setFixedSize(QSize(100, 30));
    btLayout->addWidget(m_btRandomPlay, 1);
    btLayout->addStretch();

    AC_SET_OBJECT_NAME(m_btPlayAll, AC_dialogPlayAll);
    AC_SET_ACCESSIBLE_NAME(m_btPlayAll, AC_dialogPlayAll);
    AC_SET_OBJECT_NAME(m_btRandomPlay, AC_dialogPlayRandom);
    AC_SET_ACCESSIBLE_NAME(m_btRandomPlay, AC_dialogPlayRandom);

    m_titleLabel->setContentsMargins(0, 0, 0, 0);

    m_musicListInfoView = new MusicListInfoView(hash);
    AC_SET_OBJECT_NAME(m_musicListInfoView, AC_musicListInfoView);
    AC_SET_ACCESSIBLE_NAME(m_musicListInfoView, AC_musicListInfoView);
    mainLayout->addWidget(m_musicListInfoView, 1);
    mainLayout->addStretch();

    connect(m_btPlayAll, &DPushButton::pressed, this, &SubSonglistWidget::slotPlayAllClicked);
    connect(m_btRandomPlay, &DPushButton::pressed, this, &SubSonglistWidget::slotPlayRandomClicked);
}

void SubSonglistWidget::resizeEvent(QResizeEvent *e)
{
    m_titleImage->setFixedWidth(this->width());

    QImage img = m_img.scaled(this->width(), 200, Qt::KeepAspectRatioByExpanding).toImage();
    QPainter pai(&img);
    pai.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
    QColor fillColor("#FFFFFF");
    fillColor.setAlphaF(0.6);
    pai.setBrush(fillColor);
    QRect rect = img.rect();
    rect.adjust(-5, 0, 5, 0);
    pai.drawRect(rect);
    m_titleImage->setPixmap(QPixmap::fromImage(img));
}

SubSonglistWidget::SubSonglistWidget(const QString &hash, QWidget *parent)
    : DFrame(parent)
    , hash(hash)
{
    AC_SET_OBJECT_NAME(this, AC_subSonglistWidget);
    AC_SET_ACCESSIBLE_NAME(this, AC_subSonglistWidget);
    initUI();
}

SubSonglistWidget::~SubSonglistWidget()
{

}

void SubSonglistWidget::setThemeType(int type)
{
    m_themeType = type;

    if (type == 1) {
        auto palette = this->palette();
        QColor BackgroundColor("#F8F8F8");
        palette.setColor(DPalette::Background, BackgroundColor);
        setPalette(palette);

//        auto titleLabelPl = d->titleLabel->palette();
//        titleLabelPl.setColor(DPalette::WindowText, Qt::black);
//        d->titleLabel->setPalette(titleLabelPl);

//        auto infoLabelPl = d->infoLabel->palette();
//        infoLabelPl.setColor(DPalette::WindowText, Qt::black);
//        d->infoLabel->setPalette(infoLabelPl);

        auto playAllPalette = m_btPlayAll->palette();
        playAllPalette.setColor(DPalette::ButtonText, Qt::white);
        playAllPalette.setColor(DPalette::Dark, QColor("#FD5E5E"));
        playAllPalette.setColor(DPalette::Light, QColor("#FD5E5E"));
        QColor sbcolor("#F82C47");
        sbcolor.setAlphaF(0.4);
        playAllPalette.setColor(DPalette::Shadow, sbcolor);
        m_btPlayAll->setPalette(playAllPalette);

        auto randomPlayPalette = m_btRandomPlay->palette();
        randomPlayPalette.setColor(DPalette::ButtonText, Qt::white);
        randomPlayPalette.setColor(DPalette::Dark, QColor("#646464"));
        randomPlayPalette.setColor(DPalette::Light, QColor("#5C5C5C"));
        QColor randombcolor("#000000");
        randombcolor.setAlphaF(0.2);
        randomPlayPalette.setColor(DPalette::Shadow, randombcolor);
        m_btRandomPlay->setPalette(randomPlayPalette);
    } else {
        auto palette = this->palette();
        QColor BackgroundColor("#252525");
        palette.setColor(DPalette::Background, BackgroundColor);
        setPalette(palette);

//        d->closeBt->setNormalPic(":/mpimage/dark/close_round/close_normal_dark.svg");
//        d->closeBt->setHoverPic(":/mpimage/dark/close_round/close_hover_dark.svg");
//        d->closeBt->setPressPic(":/mpimage/dark/close_round/close_press.svg");

//        auto titleLabelPl = d->titleLabel->palette();
//        titleLabelPl.setColor(DPalette::WindowText, Qt::white);
//        d->titleLabel->setPalette(titleLabelPl);

//        auto infoLabelPl = d->infoLabel->palette();
//        infoLabelPl.setColor(DPalette::WindowText, Qt::white);
//        d->infoLabel->setPalette(infoLabelPl);

        auto playAllPalette = m_btPlayAll->palette();
        playAllPalette.setColor(DPalette::ButtonText, "#FFFFFF");
        playAllPalette.setColor(DPalette::Dark, QColor("#DA2D2D"));
        playAllPalette.setColor(DPalette::Light, QColor("#A51B1B"));
        QColor sbcolor("#C10A0A");
        sbcolor.setAlphaF(0.5);
        playAllPalette.setColor(DPalette::Shadow, sbcolor);
        m_btPlayAll->setPalette(playAllPalette);

        auto randomPlayPalette = m_btRandomPlay->palette();
        randomPlayPalette.setColor(DPalette::ButtonText, "#FFFFFF");
        randomPlayPalette.setColor(DPalette::Dark, QColor("#555454"));
        randomPlayPalette.setColor(DPalette::Light, QColor("#414141"));
//        QColor randombcolor("#FFFFFF");
//        randombcolor.setAlphaF(0.08);
//        randomPlayPalette.setColor(DPalette::Shadow, randombcolor);
        m_btRandomPlay->setPalette(randomPlayPalette);
    }
    QString rStr;
    if (type == 1) {
        rStr = "light";
    } else {
        rStr = "dark";
    }
    m_btPlayAll->setIcon(QIcon(QString(":/mpimage/light/normal/play_all_normal.svg").arg(rStr)));
    m_btRandomPlay->setIcon(QIcon(QString(":/mpimage/light/normal/random_play_normal.svg").arg(rStr)));
//    infoDialog->setThemeType(type);
}

void SubSonglistWidget::flushDialog(QMap<QString, MediaMeta> musicinfos, int isAlbumDialog)
{
    if (musicinfos.size() > 0) {
        auto titleFont = m_titleLabel->font();
        auto infoFont = m_infoLabel->font();

        if (isAlbumDialog) {
            titleFont.setPixelSize(24);
            infoFont.setPixelSize(18);
            m_titleLabel->setText(musicinfos.first().album);
            m_infoLabel->setText(musicinfos.first().singer);
        } else {
            titleFont.setPixelSize(36);
            m_titleLabel->setText(musicinfos.first().singer);
            m_infoLabel->hide();
        }

        DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T3, QFont::DemiBold);
        m_infoLabel->setFont(infoFont);
        DFontSizeManager::instance()->bind(m_infoLabel, DFontSizeManager::T5, QFont::Normal);
        m_titleLabel->setForegroundRole(DPalette::TextTitle);

        // 设置titleImage
        QPixmap img;
        for (auto meta : musicinfos) {
            QString imagesDirPath = Global::cacheDir() + "/images/" + meta.hash + ".jpg";
            // 查找可用图片
            if (QFileInfo(imagesDirPath).exists()) {
                img = QPixmap(imagesDirPath);
                break;
            }
        }

        if (img.isNull()) {
            img = QPixmap(":/common/image/cover_max.svg");
        }

        setTitleImage(img);

        m_musicListInfoView->setMusicListView(musicinfos);
    }
}

void SubSonglistWidget::slotPlayAllClicked()
{
    QList<MediaMeta> musicList = m_musicListInfoView->getMusicListData();
    if (musicList.size() > 0) {
        emit CommonService::getInstance()->signalSetPlayModel(Player::RepeatAll);
        Player::getInstance()->setCurrentPlayListHash(hash, false);
        Player::getInstance()->setPlayList(musicList);
        Player::getInstance()->playMeta(musicList.first());
        emit Player::getInstance()->signalPlayListChanged();
    }
}

void SubSonglistWidget::slotPlayRandomClicked()
{
    QList<MediaMeta> musicList = m_musicListInfoView->getMusicListData();
    if (musicList.size() > 0) {
        Player::getInstance()->setPlayList(musicList);
        emit CommonService::getInstance()->signalSetPlayModel(Player::Shuffle);
        Player::getInstance()->playNextMeta(false);

        Player::getInstance()->setCurrentPlayListHash(hash, false);
        emit Player::getInstance()->signalPlayListChanged();
    }
}

void SubSonglistWidget::setTitleImage(QPixmap &img)
{
    m_img = img;
    img = img.scaled(this->width(), 200, Qt::KeepAspectRatioByExpanding);

    QPainter pai(&img);
    pai.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

    QColor fillColor("#FFFFFF");
    fillColor.setAlphaF(0.6);
    pai.setBrush(fillColor);
    QRect rect = img.rect();
    rect.adjust(-5, 0, 5, 0);
    pai.drawRect(rect);

    m_titleImage->setPixmap(img);
}


