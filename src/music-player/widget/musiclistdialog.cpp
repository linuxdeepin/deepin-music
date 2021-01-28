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

#include "musiclistdialog.h"

#include <QDebug>
#include <QFileInfo>
#include <DPalette>

#include <DFrame>
#include <DLabel>
#include <QBitmap>
#include <DPushButton>
#include <DApplicationHelper>
#include <DHiDPIHelper>

#include "global.h"
#include "commonservice.h"
#include "dialogList/musiclistinfoview.h"
#include "ac-desktop-define.h"
#include "player.h"

DWIDGET_USE_NAMESPACE

void MusicListDialog::initUI()
{
    setAutoFillBackground(true);
    auto palette = this->palette();
    QColor BackgroundColor("#F8F8F8");
    palette.setColor(DPalette::Window, BackgroundColor);
    setPalette(palette);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 10, 0, 0);


    m_titleImage = new DLabel;
    m_titleImage->setFixedSize(480, 130);
    m_titleImage->setForegroundRole(DPalette::BrightText);

    QPixmap m_bgImage = DHiDPIHelper::loadNxPixmap(":/common/image/cover_max.svg");
    setTitleImage(m_bgImage);


    auto closeLayout = new QHBoxLayout(m_titleImage);
    closeLayout->setSpacing(0);
    closeLayout->setContentsMargins(0, 5, 5, 0);

    auto titleLayout = new QVBoxLayout(m_titleImage);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(28, 10, 0, 18);

    m_closeBt = new DIconButton(this);
    m_closeBt->setIcon(QIcon::fromTheme("text_close_round"));
    m_closeBt->setEnabledCircle(true);
    m_closeBt->setIconSize(QSize(20, 20));
    m_closeBt->setFixedSize(24, 24);

    m_titleLabel = new DLabel();
    m_titleLabel->setForegroundRole(DPalette::BrightText);

    m_infoLabel = new DLabel();
    m_infoLabel->setForegroundRole(DPalette::BrightText);

    auto textLayout = new QVBoxLayout(m_titleImage);
    textLayout->setSpacing(0);
    textLayout->setContentsMargins(0, 5, 0, 5);

    auto btLayout = new QHBoxLayout(m_titleImage);
    btLayout->setSpacing(0);
    btLayout->setContentsMargins(0, 0, 0, 0);

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
    m_btPlayAll->setFixedHeight(30);
    m_btPlayAll->setFocusPolicy(Qt::NoFocus);
    m_btPlayAll->setIconSize(QSize(18, 18));

    m_btRandomPlay = new DPushButton;
    m_btRandomPlay->setFont(btPlayAllFont);
    auto randomPlayPalette = m_btRandomPlay->palette();
    randomPlayPalette.setColor(DPalette::ButtonText, Qt::white);
    randomPlayPalette.setColor(DPalette::Dark, QColor(Qt::darkGray));
    randomPlayPalette.setColor(DPalette::Light, QColor(Qt::darkGray));
    m_btRandomPlay->setPalette(randomPlayPalette);
    m_btRandomPlay->setIcon(QIcon(":/mpimage/light/normal/random_play_normal.svg"));
    m_btRandomPlay->setText(MusicListDialog::tr("Shuffle"));
    m_btRandomPlay->setFixedHeight(30);
    m_btRandomPlay->setFocusPolicy(Qt::NoFocus);
    m_btRandomPlay->setIconSize(QSize(18, 18));

    btLayout->addWidget(m_btPlayAll);
    btLayout->addSpacing(10);
    btLayout->addWidget(m_btRandomPlay);
    btLayout->addStretch(100);

//    AC_SET_OBJECT_NAME(m_btPlayAll, AC_dialogPlayAll);
//    AC_SET_ACCESSIBLE_NAME(m_btPlayAll, AC_dialogPlayAll);
    AC_SET_OBJECT_NAME(m_btRandomPlay, AC_dialogPlayRandom);
    AC_SET_ACCESSIBLE_NAME(m_btRandomPlay, AC_dialogPlayRandom);


//    titleLayout->addWidget(closeBt, 0, Qt::AlignTop | Qt::AlignRight);
    m_titleLabel->setContentsMargins(0, 0, 0, 0);
    textLayout->addWidget(m_titleLabel, 0, Qt::AlignVCenter);
    textLayout->addWidget(m_infoLabel, 0, Qt::AlignVCenter);
    titleLayout->addLayout(textLayout, 1);
    titleLayout->addLayout(btLayout, 0);

    closeLayout->addLayout(titleLayout);
    closeLayout->addWidget(m_closeBt, 0, Qt::AlignTop | Qt::AlignRight);

    AC_SET_OBJECT_NAME(m_closeBt, AC_musicListDialogCloseBt);
    AC_SET_ACCESSIBLE_NAME(m_closeBt, AC_musicListDialogCloseBt);

//    m_musicListInfoView = new MusicListInfoView(hash);

    layout->addWidget(m_titleImage, 0, Qt::AlignTop | Qt::AlignCenter);
//    layout->addWidget(m_musicListInfoView);

    // 需求改动，这个页面不再使用，标签先注释，后期删除这个文件
//    AC_SET_OBJECT_NAME(m_musicListInfoView, AC_musicListInfoView);
//    AC_SET_ACCESSIBLE_NAME(m_musicListInfoView, AC_musicListInfoView);

//    infoDialog = new InfoDialog(q);
//    infoDialog->hide();

    connect(m_btPlayAll, &DPushButton::pressed, this, &MusicListDialog::slotPlayAllClicked);
    connect(m_btRandomPlay, &DPushButton::pressed, this, &MusicListDialog::slotPlayRandomClicked);
    connect(m_closeBt, &DIconButton::pressed, this, &DAbstractDialog::close);
}

MusicListDialog::MusicListDialog(const QString &hash, QWidget *parent)
    : DAbstractDialog(parent)
    , hash(hash)
{
    resize(500, 500);

    initUI();
}

MusicListDialog::~MusicListDialog()
{

}

void MusicListDialog::setThemeType(int type)
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

void MusicListDialog::flushDialog(QMap<QString, MediaMeta> musicinfos, int isAlbumDialog)
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

void MusicListDialog::slotPlayAllClicked()
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

void MusicListDialog::slotPlayRandomClicked()
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

void MusicListDialog::setTitleImage(QPixmap &img)
{
    img = img.scaled(480, 130, Qt::KeepAspectRatioByExpanding);

    QPainter pai(&img);
    pai.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

    QColor fillColor("#FFFFFF");
    if (m_themeType != 1) {
        fillColor = QColor("#000000");
    }
    fillColor.setAlphaF(0.3);
    pai.setBrush(fillColor);

    QColor penColor("#000000");
    penColor.setAlphaF(0.08);
    QPen pen(penColor, 2);
    pai.setPen(pen);
    pai.drawRect(img.rect());


    QBitmap bmp(m_titleImage->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawRoundedRect(bmp.rect().adjusted(0, 0, 0, 5), 9, 9);

    m_titleImage->setPixmap(img);
    m_titleImage->setMask(bmp);
}


