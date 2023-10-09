// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "subsonglistwidget.h"

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
#include "ac-desktop-define.h"
#include "player.h"
#include "playlistview.h"
#include "metadetector.h"
DWIDGET_USE_NAMESPACE

SubSonglistWidget::SubSonglistWidget(const QString &hash, QWidget *parent)
    : DWidget(parent)
    , m_hash(hash)
{
    AC_SET_OBJECT_NAME(this, AC_subSonglistWidget);
    AC_SET_ACCESSIBLE_NAME(this, AC_subSonglistWidget);
    initUI();

    setThemeType(DGuiApplicationHelper::instance()->themeType());

    connect(m_btPlayAll, &DPushButton::pressed, this, &SubSonglistWidget::slotPlayAllClicked);
    connect(m_btRandomPlay, &DPushButton::pressed, this, &SubSonglistWidget::slotPlayRandomClicked);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &SubSonglistWidget::setThemeType);
#ifdef DTKWIDGET_CLASS_DSizeMode
    slotSizeModeChanged(DGuiApplicationHelper::instance()->sizeMode());
    connect(DGuiApplicationHelper::instance(),&DGuiApplicationHelper::sizeModeChanged,this, &SubSonglistWidget::slotSizeModeChanged);
#endif
}

void SubSonglistWidget::initUI()
{
    this->setFocusPolicy(Qt::ClickFocus);
    this->setAutoFillBackground(true);

    QPalette palette = this->palette();
    QColor BackgroundColor("#F8F8F8");
    palette.setColor(DPalette::Window, BackgroundColor);
    setPalette(palette);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_titleImage = new QLabel(this);
    m_titleImage->setForegroundRole(DPalette::BrightText);
    m_titleImage->setFixedHeight(150);
    QPixmap m_bgImage = QIcon::fromTheme("cover_max").pixmap(QSize(200, 200));
    setTitleImage(m_bgImage);
    mainLayout->addWidget(m_titleImage, 0);

    QVBoxLayout *titleLayout = new QVBoxLayout(m_titleImage);
    titleLayout->setSpacing(5);
    titleLayout->setContentsMargins(28, 15, 0, 20);

    m_titleLabel = new DLabel(this);
    m_titleLabel->setForegroundRole(DPalette::TextTitle);
    titleLayout->addWidget(m_titleLabel, 1);
    m_infoLabel = new DLabel(this);
    m_infoLabel->setForegroundRole(DPalette::TextTitle);
    titleLayout->addWidget(m_infoLabel, 1);
    QHBoxLayout *btLayout = new QHBoxLayout(this);
    btLayout->setSpacing(5);
    btLayout->setContentsMargins(0, 16, 0, 0);
    titleLayout->addLayout(btLayout, 1);

    // 播放全部
    m_btPlayAll = new DPushButton(this);
    auto playAllPalette = m_btPlayAll->palette();
    playAllPalette.setColor(DPalette::ButtonText, Qt::white);
    // 按钮颜色不对，由UI确认
    playAllPalette.setColor(DPalette::Dark, QColor("#FD5E5E"));
    playAllPalette.setColor(DPalette::Light, QColor("#ED5656"));
    m_btPlayAll->setPalette(playAllPalette);
    m_btPlayAll->setIcon(QIcon::fromTheme("play_all"));
    m_btPlayAll->setText(tr("Play All"));
    m_btPlayAll->setFocusPolicy(Qt::NoFocus);
    m_btPlayAll->setIconSize(QSize(18, 18));
    // 按钮自适应字体大小
    int btnWidth = CommonService::getInstance()->isTabletEnvironment() ? 100 : 93;
    int btnHight = CommonService::getInstance()->isTabletEnvironment() ? 40 : 30;
    m_btPlayAll->setMinimumSize(QSize(btnWidth, btnHight));
    // 字体宽度加图标大于93,重新设置按钮宽度
    QFontMetrics font(m_btPlayAll->font());
    m_btPlayAll->setMinimumWidth((font.width(m_btPlayAll->text()) + 18) >= btnWidth ? (font.width(m_btPlayAll->text()) + 38) : btnWidth);
    DFontSizeManager::instance()->bind(m_btPlayAll, DFontSizeManager::T6, QFont::Medium);
    btLayout->addWidget(m_btPlayAll);

    // 随机播放
    m_btRandomPlay = new DPushButton(this);
    auto randomPlayPalette = m_btRandomPlay->palette();
    randomPlayPalette.setColor(DPalette::ButtonText, Qt::white);
    randomPlayPalette.setColor(DPalette::Dark, QColor(Qt::darkGray));
    randomPlayPalette.setColor(DPalette::Light, QColor(Qt::darkGray));
    m_btRandomPlay->setPalette(randomPlayPalette);
    m_btRandomPlay->setIcon(QIcon::fromTheme("random_play"));
    m_btRandomPlay->setText(tr("Shuffle"));
    m_btRandomPlay->setFocusPolicy(Qt::NoFocus);
    m_btRandomPlay->setIconSize(QSize(18, 18));
    // 按钮自适应字体大小
    m_btRandomPlay->setMinimumSize(QSize(btnWidth, btnHight));
    // 字体宽度加图标大于93,重新设置按钮宽度
    QFontMetrics fontRandom(m_btRandomPlay->font());
    m_btRandomPlay->setMinimumWidth((fontRandom.width(m_btRandomPlay->text()) + 18) >= btnWidth ? (fontRandom.width(m_btRandomPlay->text()) + 38) : btnWidth);
    DFontSizeManager::instance()->bind(m_btRandomPlay, DFontSizeManager::T6, QFont::Medium);
    btLayout->addWidget(m_btRandomPlay);
    btLayout->addStretch();

    AC_SET_OBJECT_NAME(m_btPlayAll, AC_dialogPlayAll);
    AC_SET_ACCESSIBLE_NAME(m_btPlayAll, AC_dialogPlayAll);
    AC_SET_OBJECT_NAME(m_btRandomPlay, AC_dialogPlayRandom);
    AC_SET_ACCESSIBLE_NAME(m_btRandomPlay, AC_dialogPlayRandom);

    m_titleLabel->setContentsMargins(0, 0, 0, 0);

    m_musicListInfoView = new PlayListView(m_hash, false, false, this);
    mainLayout->addWidget(m_musicListInfoView, 1);
    mainLayout->addStretch();
    AC_SET_OBJECT_NAME(m_musicListInfoView, AC_musicListInfoView);
    AC_SET_ACCESSIBLE_NAME(m_musicListInfoView, AC_musicListInfoView);
}

void SubSonglistWidget::resizeEvent(QResizeEvent *e)
{
    m_titleImage->setFixedWidth(this->width());
    // 设置右侧显示省略号
    QFontMetrics extraNameFm(m_titleLabel->font());
    QString title = extraNameFm.elidedText(m_title, Qt::ElideRight, this->width() - 56);
    m_titleLabel->setText(title);

    setTitleImage(m_img);
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

        m_titleLabel->setForegroundRole(DPalette::TextTitle);
        m_infoLabel->setForegroundRole(DPalette::TextTitle);

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

        m_titleLabel->setForegroundRole(DPalette::HighlightedText);
        m_infoLabel->setForegroundRole(DPalette::HighlightedText);

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
        m_btRandomPlay->setPalette(randomPlayPalette);
    }
    m_btPlayAll->setIcon(QIcon::fromTheme("play_all"));
    m_btRandomPlay->setIcon(QIcon::fromTheme("random_play"));
    setTitleImage(m_img);
}

void SubSonglistWidget::flushDialog(QMap<QString, MediaMeta> musicinfos, ListPageSwitchType listPageType)
{
    if (musicinfos.size() > 0) {
        auto titleFont = m_titleLabel->font();
        auto infoFont = m_infoLabel->font();
        // 根据进入二级页面场景，设置hash值，二级页面删除时判断使用
        switch (listPageType) {
        case AlbumSubSongListType: {
            m_hash = "album";
            break;
        }
        case SearchAlbumResultType: {
            m_hash = "albumResult";
            break;
        }
        case SingerSubSongListType: {
            m_hash = "artist";
            break;
        }
        case SearchSingerResultType: {
            m_hash = "artistResult";
            break;
        }
        default: {
            m_hash = "album";
            break;
        }
        }
        m_musicListInfoView->setListPageSwitchType(listPageType);
        if (listPageType == AlbumSubSongListType || listPageType == SearchAlbumSubSongListType) {
            m_titleLabel->setText(musicinfos.first().album);
            m_title = musicinfos.first().album;
            m_infoLabel->setText(musicinfos.first().singer);
            m_infoLabel->show();
        } else if (listPageType == SingerSubSongListType || listPageType == SearchSingerSubSongListType) {
            m_titleLabel->setText(musicinfos.first().singer);
            m_title = musicinfos.first().singer;
            m_infoLabel->hide();
        }

        DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T3, QFont::DemiBold);
        m_infoLabel->setFont(infoFont);
        DFontSizeManager::instance()->bind(m_infoLabel, DFontSizeManager::T5, QFont::Normal);

        // 设置titleImage
        QPixmap img;
        for (auto meta : musicinfos) {
            QString imagesDirPath = Global::cacheDir() + "/images/" + meta.hash + ".jpg";
            // 查找可用图片
            if (QFileInfo(imagesDirPath).exists()) {
                img = MetaDetector::getCoverDataPixmap(meta, Global::playbackEngineType());
                break;
            }
        }

        if (img.isNull()) {
            img = QIcon::fromTheme("cover_max").pixmap(QSize(200, 200));
        }

        setTitleImage(img);
        if (listPageType == AlbumSubSongListType) {
            m_musicListInfoView->setMusicListView(musicinfos, "album");
        } else if (listPageType == SingerSubSongListType) {
            m_musicListInfoView->setMusicListView(musicinfos, "artist");
        } else {
            m_musicListInfoView->setMusicListView(musicinfos, "all");
        }
    }
}

void SubSonglistWidget::slotPlayAllClicked()
{
    QList<MediaMeta> musicList = m_musicListInfoView->getMusicListData();
    if (musicList.size() > 0) {
        // 清空播放队列
        Player::getInstance()->clearPlayList();
        Player::getInstance()->setCurrentPlayListHash(m_hash, false);
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

        Player::getInstance()->setCurrentPlayListHash(m_hash, false);
        emit Player::getInstance()->signalPlayListChanged();
    }
}

#ifdef DTKWIDGET_CLASS_DSizeMode
void SubSonglistWidget::slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode)
{
    if (sizeMode == DGuiApplicationHelper::SizeMode::CompactMode) {
        m_btPlayAll->setIconSize(QSize(14, 14));
        m_btPlayAll->setFixedHeight(24);
        m_btRandomPlay->setIconSize(QSize(14, 14));
        m_btRandomPlay->setFixedHeight(24);
    } else {
        m_btPlayAll->setIconSize(QSize(18, 18));
        m_btPlayAll->setFixedHeight(30);
        m_btRandomPlay->setIconSize(QSize(18, 18));
        m_btRandomPlay->setFixedHeight(30);
    }
}
#endif

void SubSonglistWidget::setTitleImage(QPixmap &img)
{
    m_img = img;

    QPixmap curImg;
    // 添加非空判断，减少警告日志输出与不必要的性能损耗
    if (!img.isNull()) {
        curImg = img.scaled(static_cast<int>(static_cast<double>(this->width()) * qApp->devicePixelRatio()), 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }

    QPainter pai(&curImg);
    pai.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

    QColor fillColor("#FFFFFF");
    if (DGuiApplicationHelper::instance()->themeType() != 1)
        fillColor = QColor(Qt::black);
    fillColor.setAlphaF(0.6);
    pai.setBrush(fillColor);
    QRect rect = curImg.rect();
    rect.adjust(-5, 0, 5, 0);
    pai.drawRect(rect);
    pai.end();

    m_titleImage->setPixmap(curImg);
}
