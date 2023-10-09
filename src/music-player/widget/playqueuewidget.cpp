// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "playqueuewidget.h"

#include <DPushButton>
#include <DComboBox>
#include <DLabel>
#include <DPalette>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <DApplicationHelper>

#include <QDebug>
#include <QAction>
#include <QVBoxLayout>
#include <QMimeData>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QApplication>

#include "../core/music.h"
#include "../core/musicsettings.h"
#include "playlistview.h"
#include "widget/ddropdown.h"
#include "../core/player.h"
#include "footerwidget.h"
#include "commonservice.h"
#include "mainframe.h"
#include "ac-desktop-define.h"

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

PlayQueueWidget::PlayQueueWidget(QWidget *parent) :
    DFloatingWidget(parent)
{
    this->setAcceptDrops(true);

    this->setBlurBackgroundEnabled(true);
    this->blurBackground()->setRadius(30);
    this->blurBackground()->setBlurEnabled(true);
    this->blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);

    m_actionBar = new DWidget(this);
    m_actionBar->setFixedWidth(200);
    m_actionBar->setObjectName("PlayListActionBar");

    m_titleLabel = new DLabel(this);
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T3);
    m_titleLabel->setMargin(4);
    m_titleLabel->setText(tr("Play Queue"));
    m_titleLabel->setForegroundRole(DPalette::BrightText);

    m_infoLabel = new DLabel(this);
    DFontSizeManager::instance()->bind(m_infoLabel, DFontSizeManager::T6);
    m_infoLabel->setMargin(4);
    m_infoLabel->setForegroundRole(DPalette::ButtonText);

    m_btClearAll = new DPushButton(this);
    m_btClearAll->setIcon(QIcon::fromTheme("clear_list"));
    DFontSizeManager::instance()->bind(m_btClearAll, DFontSizeManager::T6);
    m_btClearAll->setObjectName("PlayListPlayAll");
    m_btClearAll->setText(tr("Empty"));
    int btnWidth = CommonService::getInstance()->isTabletEnvironment() ? 100 : 93;
    int btnHight = CommonService::getInstance()->isTabletEnvironment() ? 40 : 30;
    m_btClearAll->setMinimumSize(QSize(btnWidth, btnHight));

    m_btClearAll->setFocusPolicy(Qt::TabFocus);
    m_btClearAll->setDefault(true);

    m_emptyHits = new DLabel(this);
    m_emptyHits->setText(tr("No songs"));
    m_emptyHits->setObjectName("PlayListEmptyHits");
    m_emptyHits->setVisible(false);


    QVBoxLayout *actionBarLayout = new QVBoxLayout(m_actionBar);
    actionBarLayout->setContentsMargins(36, 0, 8, 0);
    actionBarLayout->setSpacing(0);
    actionBarLayout->addWidget(m_titleLabel);
    actionBarLayout->addWidget(m_infoLabel);
    actionBarLayout->addWidget(m_btClearAll, 0, Qt::AlignLeft);
    actionBarLayout->addStretch();

    m_playListView = new PlayListView("play", true, true, this);
    m_playListView->setFocusPolicy(Qt::StrongFocus);

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_actionBar, 0);
    m_mainLayout->addWidget(m_playListView, 1);
    m_mainLayout->addWidget(m_emptyHits, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(12);
    m_mainLayout->setContentsMargins(0, 30, 0, 70);

    initAnimation();

    connect(m_btClearAll, &DPushButton::clicked, this, &PlayQueueWidget::slotClearAllClicked);
    connect(m_playListView, &PlayListView::rowCountChanged, this, &PlayQueueWidget::slotUpdateItemCount);
    // 初始化播放列表数据
    this->slotPlayListChanged();
    connect(Player::getInstance(), &Player::signalPlayListChanged, this, &PlayQueueWidget::slotPlayListChanged);
    connect(qApp, &QApplication::focusChanged, this, &PlayQueueWidget::autoHidden);

    connect(DataBaseService::getInstance(), &DataBaseService::signalAllMusicCleared,
            this, &PlayQueueWidget::slotAllMusicCleared);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &PlayQueueWidget::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());
#ifdef DTKWIDGET_CLASS_DSizeMode
    slotSizeModeChanged(DGuiApplicationHelper::instance()->sizeMode());
    connect(DGuiApplicationHelper::instance(),&DGuiApplicationHelper::sizeModeChanged,this, &PlayQueueWidget::slotSizeModeChanged);
#endif

    this->hide();
}

void PlayQueueWidget::playAnimation(const QSize &size)
{
    QRect startRect = QRect(Margin, size.height() - Height - Margin, size.width() - Margin * 2, Height);
    QRect endRect = QRect(Margin, size.height() - 450, size.width() - Margin * 2, 445);

    m_animationToUp->setStartValue(startRect);
    m_animationToUp->setEndValue(endRect);

    m_animationToDown->setStartValue(endRect);
    m_animationToDown->setEndValue(startRect);

    if (this->isHidden()) {
        this->show();
        m_animationToUp->start();
    } else {
        this->show();
        m_animationToDown->start();
    }
}

void PlayQueueWidget::stopAnimation()
{
    m_animationToDown->stop();
}

void PlayQueueWidget::initAnimation()
{
    // 动画状态Up,初始化指明父类，方便后续释放，防止内存泄露
    m_animationToUp = new QPropertyAnimation(this, "geometry", this);
    m_animationToUp->setEasingCurve(QEasingCurve::InCurve);
    m_animationToUp->setDuration(AnimationDelay);

    // 动画状态Down,初始化指明父类，方便后续释放，防止内存泄露
    m_animationToDown = new QPropertyAnimation(this, "geometry", this);
    m_animationToDown->setEasingCurve(QEasingCurve::InCurve);
    m_animationToDown->setDuration(AnimationDelay);
    m_animationToDown->connect(m_animationToDown, &QPropertyAnimation::finished, this, [ = ]() {
        this->hide();

        emit signalAutoHidden();
        emit CommonService::getInstance()->signalPlayQueueClosed();
    });
}

void PlayQueueWidget::slotPlayListChanged()
{
    m_playListView->playListChange();

    if (m_playListView->getMusicListData().size() == 0) {
        m_emptyHits->setVisible(true);
        m_playListView->setVisible(false);
    } else {
        m_emptyHits->setVisible(false);
        m_playListView->setVisible(true);
    }
}

PlayQueueWidget::~PlayQueueWidget()
{
}

void PlayQueueWidget::dragEnterEvent(QDragEnterEvent *event)
{
    DWidget::dragEnterEvent(event);
    if (event->mimeData()->hasFormat("text/uri-list")) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return;
    }
}

void PlayQueueWidget::dropEvent(QDropEvent *event)
{
    DWidget::dropEvent(event);

    if (!event->mimeData()->hasFormat("text/uri-list")) {
        return;
    }

    auto urls = event->mimeData()->urls();
    QStringList localpaths;
    for (auto &url : urls) {
        localpaths << (url.isLocalFile() ? url.toLocalFile() : url.path());
    }

//    if (!localpaths.isEmpty() && !m_playListView->playlist().isNull()) {
//        Q_EMIT importSelectFiles(m_playListView->playlist(), localpaths);
//    }
}

void PlayQueueWidget::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
}

void PlayQueueWidget::setThemeType(int type)
{
    if (type == 1) {
        QColor backMaskColor(247, 247, 247);
        // 按设计修改透明度
        backMaskColor.setAlphaF(0.9);
        this->blurBackground()->setMaskColor(backMaskColor);

//        auto titleLabelPalette = d->infoLabel->palette();
//        titleLabelPalette.setColor(DPalette::ButtonText, ("#000000"));
//        d->titleLabel->setPalette(titleLabelPalette);
//        d->titleLabel->setForegroundRole(DPalette::ButtonText);

        auto infoLabelPalette = m_infoLabel->palette();
        QColor infoLabelColor("#000000");
        infoLabelColor.setAlphaF(0.5);
        infoLabelPalette.setColor(DPalette::ButtonText, infoLabelColor);
        DApplicationHelper::instance()->setPalette(m_infoLabel, infoLabelPalette);

        DPalette pl = m_btClearAll ->palette();
        pl.setColor(DPalette::ButtonText, QColor("#FFFFFF"));
        pl.setColor(DPalette::Light, QColor("#646464"));
        pl.setColor(DPalette::Dark, QColor("#5C5C5C"));
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.08);
        pl.setColor(DPalette::Shadow, sbcolor);
        m_btClearAll->setPalette(pl);
    } else {
        QColor backMaskColor(32, 32, 32);
        // 按设计修改透明度
        backMaskColor.setAlphaF(0.9);
        this->blurBackground()->setMaskColor(backMaskColor);

//        auto titleLabelPalette = d->infoLabel->palette();
//        titleLabelPalette.setColor(DPalette::ButtonText, ("#FFFFFF"));
//        d->titleLabel->setPalette(titleLabelPalette);
//        d->titleLabel->setForegroundRole(DPalette::ButtonText);

        auto infoLabelPalette = m_infoLabel->palette();
        QColor infoLabelColor("#ffffff");
        infoLabelColor.setAlphaF(0.5);
        infoLabelPalette.setColor(DPalette::ButtonText, infoLabelColor);
        DApplicationHelper::instance()->setPalette(m_infoLabel, infoLabelPalette);

        DPalette pl = m_btClearAll->palette();
        pl.setColor(DPalette::ButtonText, QColor("#FFFFFF"));
        pl.setColor(DPalette::Light, QColor("#555454"));
        pl.setColor(DPalette::Dark, QColor("#414141"));
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.08);
        pl.setColor(DPalette::Shadow, sbcolor);
        m_btClearAll->setPalette(pl);
    }

    m_playListView->setThemeType(type);
}

#ifdef DTKWIDGET_CLASS_DSizeMode
void PlayQueueWidget::slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode)
{
    if (sizeMode == DGuiApplicationHelper::SizeMode::CompactMode) {
        m_mainLayout->setContentsMargins(0, 20, 0, 40);
        this->setContentsMargins(3, 3, 3, 3);
    } else {
        m_mainLayout->setContentsMargins(0, 30, 0, 70);
        this->setContentsMargins(6, 6, 6, 6);
    }
}
#endif

void PlayQueueWidget::slotClearAllClicked()
{
    Player::getInstance()->clearPlayList();
    emit Player::getInstance()->signalPlayListChanged();
}

void PlayQueueWidget::slotUpdateItemCount()
{
    QString infoStr;
    int inum = m_playListView->model()->rowCount();
    if (inum == 0) {
        infoStr = tr("No songs");
    } else if (inum == 1) {
        infoStr = tr("1 song");
    } else {
        infoStr = tr("%1 songs").arg(inum);
    }
    m_infoLabel->setText(infoStr);
}

void PlayQueueWidget::autoHidden(QWidget *old, QWidget *now)
{
    if (old && now) {
        if (!(now->objectName() == "infoDialog" || now->objectName() == "InfoTitle" ||
                now->objectName() == "MainFrame" || now->objectName() == "messageBox" || now->objectName() == AC_customizeListview))  {

            PlayListView *playListQueue = dynamic_cast<PlayListView *>(now);

            if (playListQueue && playListQueue->objectName() == "PlayListView") {
                if (!playListQueue->getIsPlayQueue()) {
                    if (!this->isHidden()) {
                        QWidget *parent = static_cast<QWidget *>(this->parent());
                        if (parent) {
                            this->playAnimation(parent->size());
                        }
                    }
                }
            } else {
                PlayListView *playListQueueOld = dynamic_cast<PlayListView *>(old);
                if ((playListQueueOld
                        && old->objectName() == "PlayListView"
                        && now->objectName().isEmpty()
                        && playListQueueOld->getMenuIsShow()) || (old->objectName().isEmpty()
                                                                  && now->objectName().isEmpty())) {
                    // 右键菜单正在显示或者进入二级页面时不做隐藏处理
                } else if (now->objectName() != "FooterWidget") {
                    if (!this->isHidden()) {
                        QWidget *parent = static_cast<QWidget *>(this->parent());
                        if (parent) {
                            this->playAnimation(parent->size());
                        }
                    }
                }
            }

        }
    }
}

// 解决进入导入界面播放队列未收起Bug
void PlayQueueWidget::slotAllMusicCleared()
{
    QWidget *parent = static_cast<QWidget *>(this->parent());
    if (parent) {
        this->setGeometry(0, parent->height() - 85, parent->width(), 80);
        emit CommonService::getInstance()->signalPlayQueueClosed();
        emit signalAutoHidden();
        this->hide();
    }
}

