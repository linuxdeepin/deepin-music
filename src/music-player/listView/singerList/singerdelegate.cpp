// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singerdelegate.h"
#include "singerlistview.h"
#include "core/medialibrary.h"
#include "global.h"

#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QDate>
#include <QEvent>
#include <QMouseEvent>
#include <DGuiApplicationHelper>
#include <DHiDPIHelper>
#include <QFileInfo>

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

DGUI_USE_NAMESPACE

static inline int playPauseBtnSize()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
        if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
            // 紧凑模式缩小到80%
            return 40;
        } else
#endif
        {
            return 50;
        }
}

static inline int heightTopMargin()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
        if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
            // 紧凑模式缩小到80%
            return 29;
        } else
#endif
        {
            return 36;
        }
}
static inline int pixel2point(int pixel)
{
    return pixel * 96 / 72;
}

void SingerDataDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    auto listview = qobject_cast<const SingerListView *>(option.widget);

    if (CommonService::getInstance()->isTabletEnvironment()) {
        if (listview->viewMode() == QListView::IconMode) {
            drawTabletIconMode(*painter, option, index);
        } else {
            this->drawListMode(*painter, option, index);
        }
    } else {
        if (listview->viewMode() == QListView::IconMode) {
            drawIconMode(*painter, option, index);
        } else {
            drawListMode(*painter, option, index);
        }
    }
}

QSize SingerDataDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    auto *listview = qobject_cast<const SingerListView *>(option.widget);
    if (listview && listview->viewMode() == QListView::IconMode) {
        if (CommonService::getInstance()->isTabletEnvironment()) {
            return QSize(200, 200);
        } else {
#ifdef DTKWIDGET_CLASS_DSizeMode
            if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
                return QSize(126, 126);
            } else
#endif
            {
                return QSize(150, 150);
            }
        }
    } else {
        QSize baseSize = QStyledItemDelegate::sizeHint(option, index);
#ifdef DTKWIDGET_CLASS_DSizeMode
        if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
            return QSize(baseSize.width(), 30);
        } else
#endif
        {
            return QSize(baseSize.width(), 38);
        }
    }
}

bool SingerDataDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (CommonService::getInstance()->isTabletEnvironment()) {
        const SingerListView *singerListView = qobject_cast<const SingerListView *>(option.widget);
        const QMouseEvent *pressEvent = static_cast<QMouseEvent *>(event);
        const QPointF pressPos = pressEvent->pos();

        if (index.isValid() && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)) {
            // 用于判断触屏点击状态
            static int clickedCount = 0;
            // 触屏点击次数
            clickedCount++;

            QTimer::singleShot(200, [ = ]() {
                if (clickedCount == 1) {
                    if (singerListView->viewMode() == QListView::IconMode) {
                        // IconMode触屏单击
                        touchClicked(option, index, pressPos);
                    } else if (singerListView->viewMode() == QListView::ListMode) {
                        // 同鼠标双击，直接进入二级菜单
                        mouseDoubleClicked(option, index);
                    }
                } else if (clickedCount > 1) {
                    // 触屏双击
                    touchDoubleClicked(option, index);
                }
                // 点击次数归零
                clickedCount = 0;
            });
        }
    } else {
        const SingerListView *singerListView = qobject_cast<const SingerListView *>(option.widget);
        const QMouseEvent *pressEvent = static_cast<QMouseEvent *>(event);
        const QPointF pressPos = pressEvent->pos();

        if (index.isValid() && singerListView->viewMode() == QListView::IconMode &&
                (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)) {
            // 用于判断鼠标点击状态
            static int clickedCount = 0;
            // 鼠标点击次数
            clickedCount++;

            QTimer::singleShot(200, [ = ]() {
                if (clickedCount == 1) {
                    // 鼠标单击
                    mouseClicked(option, index, pressPos);
                } else if (clickedCount > 1) {
                    // 鼠标双击
                    mouseDoubleClicked(option, index);
                }
                // 点击次数归零
                clickedCount = 0;
            });
        } else if (index.isValid() && singerListView->viewMode() == QListView::ListMode && event->type() == QEvent::MouseButtonDblClick) {
            // 鼠标双击
            mouseDoubleClicked(option, index);
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void SingerDataDelegate::drawIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const SingerListView *listview = qobject_cast<const SingerListView *>(option.widget);
    SingerInfo singertmp = index.data(Qt::UserRole).value<SingerInfo>();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QBrush background = option.palette.background();
    painter.fillRect(option.rect, background);
    // 绘制阴影
    QRect shadowRect(option.rect.x() - 10, option.rect.y(), option.rect.width() + 8, option.rect.height() + 8);
    QPainterPath roundRectShadowPath;
    roundRectShadowPath.addRoundRect(shadowRect, 8, 8);
    painter.save();
    painter.setClipPath(roundRectShadowPath);
    painter.drawPixmap(shadowRect, shadowImg);
    painter.restore();

    int borderWidth = 0;
    QRect rect = option.rect.adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth);
    QPainterPath roundRectPath;
    roundRectPath.addRoundRect(rect, 10, 10);
    painter.setClipPath(roundRectPath);
    // 画背景图片
    QIcon opticon;
    QVariant value = index.data(Qt::DecorationRole);
    if (value.type() == QVariant::Icon) {
        opticon = qvariant_cast<QIcon>(value);
    }
    painter.drawPixmap(rect, opticon.pixmap(rect.width(), rect.width()));

    // draw border
    painter.save();
    QColor borderPenColor("#000000");
    borderPenColor.setAlphaF(0.05);
    QPen borderPen(borderPenColor);
    borderPen.setWidthF(2);
    painter.setPen(borderPen);
    painter.drawRoundRect(rect, 10, 10);
    painter.restore();

    bool playFlag = singertmp.musicinfos.keys().contains(Player::getInstance()->getActiveMeta().hash);
    Player::PlaybackStatus playStatue = Player::getInstance()->status();

    QColor fillColor(0, 0, 0);
    fillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        fillColor = "#000000";
        fillColor.setAlphaF(0.3);
    }

    int blurHeight = 36;
    int playingBlurHeight = 72;
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
        blurHeight = 30;
        playingBlurHeight = 58;
    }
#endif
    int curBlurHeight = blurHeight;
    QRect fillBlurRect(rect.x(), rect.y() + rect.height() - blurHeight, rect.width(), blurHeight);

    if (playFlag && (playStatue == Player::Playing)) {
        fillBlurRect = QRect(rect.x(), rect.y() + rect.height() - playingBlurHeight, rect.width(), 76);
        curBlurHeight = playingBlurHeight;
    }

    // 设置模糊
    QImage blurImage = opticon.pixmap(rect.width(), rect.height()).toImage();
    qreal blurRatio = blurImage.devicePixelRatioF();
    curBlurHeight = static_cast<int>(curBlurHeight * blurRatio);
    blurImage  = blurImage.copy(0, rect.height() - curBlurHeight, blurImage.width(), curBlurHeight);
    QTransform old_transformBlur = painter.transform();
    painter.translate(fillBlurRect.topLeft());
    qt_blurImage(&painter, blurImage, 35, false, false);
    painter.setTransform(old_transformBlur);
    painter.fillRect(fillBlurRect, fillColor);

    // draw playing
    SingerListView *listview2 = qobject_cast<SingerListView *>(const_cast<QWidget *>(option.widget));
    if (playFlag && playStatue == Player::Playing) {
        if (option.state & QStyle::State_MouseOver) {
            painter.drawPixmap(QRect(rect.x() + (rect.width() - 36) / 2,
                                     rect.y() + rect.height() - (playingBlurHeight + (playingBlurHeight - blurHeight - 36) / 2),
                                     36, 36), hoverSuspendImg);
        } else {
            if (listview2) {
                painter.drawPixmap(QRect(rect.x() + (rect.width() - 20) / 2,
                                         rect.y() +  rect.height() - (playingBlurHeight + (playingBlurHeight - blurHeight - 50) / 2),
                                         20, 18), listview2->getPlayPixmap(true));
            }
        }
    }

    // 绘制歌手
    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    QFontMetrics fm(fontT6);
    QRect nameRect(rect.x(), rect.y() + rect.height() - blurHeight, rect.width(), blurHeight);

    painter.setFont(fontT6);
    nameRect.adjust(8, 0, -7, 0);
    QString nameText = fm.elidedText(singertmp.singerName.isEmpty() ? SingerListView::tr("Unknown artist") : singertmp.singerName, Qt::ElideMiddle, nameRect.width());
    painter.setPen(Qt::white);
    painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

    // 绘制播放按钮
    fillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        fillColor = "#000000";
    }

    if ((option.state & QStyle::State_MouseOver) && (!playFlag || playStatue == Player::Paused)) {
        painter.save();

        int blurSize = 60;
        int iconSize = 43;
#ifdef DTKWIDGET_CLASS_DSizeMode
        if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
            // 紧凑模式缩小到80%
            blurSize = 48;
            iconSize = 34;
        }
#endif

        QImage blurImage = opticon.pixmap(rect.width(), rect.height()).toImage();
        int t_ratio = static_cast<int>(blurImage.devicePixelRatioF());
        QRect imageRect((rect.width() - blurSize) / 2, (rect.height() - blurSize) / 2,
                          blurSize * t_ratio, blurSize * t_ratio);
        blurImage  = blurImage.copy(imageRect);
        QRect hoverRect(rect.x() + (rect.width() - playPauseBtnSize()) / 2, rect.y() + heightTopMargin(),
                          playPauseBtnSize() * t_ratio, playPauseBtnSize() * t_ratio);

        QTransform old_transform = painter.transform();
        painter.translate(hoverRect.topLeft());
        // 截取成圆
        QPainterPath imageClipPath;
        imageClipPath.addEllipse(QRect(0, 0, playPauseBtnSize(), playPauseBtnSize()));
        painter.setClipPath(imageClipPath);
        qt_blurImage(&painter, blurImage, 30, false, false);
        painter.setTransform(old_transform);
        painter.fillRect(hoverRect, fillColor);
        // 绘制图标
        QPixmap t_hoverPlayImg(hoverPlayImg);
        t_hoverPlayImg.setDevicePixelRatio(option.widget->devicePixelRatioF());
        QRect pixMapRect(hoverRect.x() + (hoverRect.width() - iconSize) / 2, hoverRect.y() + (hoverRect.height() - iconSize) / 2,
                         iconSize, iconSize);
        painter.drawPixmap(pixMapRect, t_hoverPlayImg);
        painter.restore();
    }

    // 选中后
    QBrush t_fillBrush(QColor(128, 128, 128, 0));
    if (option.state & QStyle::State_Selected) {
        t_fillBrush = QBrush(QColor(128, 128, 128, 90));
    }
    painter.fillRect(option.rect, t_fillBrush);
}

extern QPixmap blurPixmap(const QPixmap &pix, int radius, int tp, const QRect &clipRect);

void SingerDataDelegate::drawTabletIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto listview = qobject_cast<const SingerListView *>(option.widget);
    SingerInfo singertmp = index.data(Qt::UserRole).value<SingerInfo>();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    auto background = option.palette.background();
    painter.fillRect(option.rect, background);
    // 绘制阴影
    QRect shadowRect(option.rect.x() - 10, option.rect.y(), 158, 158);
    QPainterPath roundRectShadowPath;
    roundRectShadowPath.addRoundRect(shadowRect, 8, 8);
    painter.save();
    painter.setClipPath(roundRectShadowPath);
    painter.drawPixmap(shadowRect, shadowImg);
    painter.restore();

    int borderWidth = 0;
    QRect rect = option.rect.adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth);
    QPainterPath roundRectPath;
    roundRectPath.addRoundRect(rect, 10, 10);
    painter.setClipPath(roundRectPath);

    // 画背景图片
    QIcon opticon;
    auto value = index.data(Qt::DecorationRole);
    if (value.type() == QVariant::Icon) {
        opticon = qvariant_cast<QIcon>(value);
    }
    painter.drawPixmap(rect, opticon.pixmap(rect.width(), rect.width()));

    // draw border
    painter.save();
    QColor borderPenColor("#000000");
    borderPenColor.setAlphaF(0.05);
    QPen borderPen(borderPenColor);
    borderPen.setWidthF(2);
    painter.setPen(borderPen);
    painter.drawRoundRect(rect/*.adjusted(1, 1, -1, 1)*/, 10, 10);
    painter.restore();

    bool playFlag = singertmp.musicinfos.keys().contains(Player::getInstance()->getActiveMeta().hash);
    Player::PlaybackStatus playStatue = Player::getInstance()->status();

    QColor fillColor(0, 0, 0);
    fillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        fillColor = "#000000";
        fillColor.setAlphaF(0.3);
    }
    int startHeight = rect.y() + rect.height() - 56;
    int fillAllHeight = 57;
    //int curFillSize = fillAllHeight;
    QRect fillBlurRect(rect.x(), rect.y() + rect.height() - fillAllHeight, rect.width(), fillAllHeight);

    if (playFlag && (playStatue == Player::Playing)) {
        fillBlurRect = QRect(rect.x(), rect.y() + rect.height() - 70, rect.width(), 76);
        //curFillSize = 70;
    }

    // 设置模糊
    QPixmap cov;
    cov = opticon.pixmap(200, 200);
    cov = cov.scaled(200, 200);
    QRect target;
    auto *listview2 = qobject_cast<SingerListView *>(const_cast<QWidget *>(option.widget));
    if (playFlag) {
        // 播放状态
        fillBlurRect = QRect(option.rect.x(), option.rect.y() + 93, option.rect.width(), 107);
        // 设置模糊
        cov = blurPixmap(cov, 20, 0, QRect(0, 93, 200, 107));
        target = option.rect;
        painter.drawPixmap(target.adjusted(0, 93, 0, 0), cov, QRect(0, 0, 200, 107));
        // 绘制透明阴影
        painter.fillRect(fillBlurRect, fillColor);
        // 绘制播放动态图
        painter.drawPixmap(QRect(option.rect.x() + 89, option.rect.y() + 114, 22, 18), listview2->getPlayPixmap(true));
    } else {
        // 普通状态
        // 设置模糊
        cov = blurPixmap(cov, 20, 0, QRect(0, 143, 200, 57));
        target = option.rect;
        painter.drawPixmap(target.adjusted(0, 143, 0, 0), cov, QRect(0, 0, 200, 57));
        // 绘制透明阴影
        painter.fillRect(fillBlurRect, fillColor);
    }

    QRect fillRect(rect.x(), startHeight, rect.width(), fillAllHeight);

    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    painter.setFont(fontT6);
    QFontMetrics fm(fontT6);

    QRect nameFillRect(option.rect.x() + 8, option.rect.y() + 160, 135, 40);
    auto nameText = fm.elidedText(singertmp.singerName.isEmpty() ? SingerListView::tr("Unknown artist") : singertmp.singerName, Qt::ElideMiddle, nameFillRect.width());
    painter.setPen(Qt::white);
    painter.drawText(nameFillRect, Qt::AlignLeft | Qt::AlignTop, nameText);

    QBrush t_fillBrush(QColor(128, 128, 128, 0));

    fillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        fillColor = "#000000";
        fillColor.setAlphaF(0.3);
    }

    if (option.state & QStyle::State_Selected) {
        t_fillBrush = QBrush(QColor(128, 128, 128, 90));
    }

    painter.save();
    QImage t_image = opticon.pixmap(rect.width(), rect.height()).toImage();
    int t_ratio = static_cast<int>(t_image.devicePixelRatioF());
    QRect t_imageRect(rect.width() / 2 - 25, rect.height() / 2 - 25, 60 * t_ratio, 60 * t_ratio);
    t_image  = t_image.copy(t_imageRect);
    QRect t_hoverRect(rect.x() + 150, rect.y() + 150, 50 * t_ratio, 50 * t_ratio);

    QTransform old_transform = painter.transform();
    painter.translate(t_hoverRect.topLeft());

    QPainterPath t_imageClipPath;
    t_imageClipPath.addEllipse(QRect(0, 0, 40, 40));
    painter.setClipPath(t_imageClipPath);

    qt_blurImage(&painter, t_image, 30, false, false);
    painter.setTransform(old_transform);
    painter.fillRect(t_hoverRect, fillColor);
    if (!playFlag) {
        // 没有播放，绘制播放按钮
        QPixmap t_hoverPlayImg(hoverPlayImg);
        t_hoverPlayImg.setDevicePixelRatio(option.widget->devicePixelRatioF());
        QRect t_pixMapRect(rect.x() + 150, rect.y() + 150, 40, 40);
        painter.drawPixmap(t_pixMapRect, t_hoverPlayImg);
    } else {
        if (playStatue == Player::Paused) {
            // 暂停状态，绘制播放按钮
            QPixmap t_hoverPlayImg(hoverPlayImg);
            t_hoverPlayImg.setDevicePixelRatio(option.widget->devicePixelRatioF());
            QRect t_pixMapRect(rect.x() + 150, rect.y() + 150, 40, 40);
            painter.drawPixmap(t_pixMapRect, t_hoverPlayImg);
        } else {
            // 播放状态，绘制暂停按钮
            QPixmap t_hoverPlayImg(hoverSuspendImg);
            t_hoverPlayImg.setDevicePixelRatio(option.widget->devicePixelRatioF());
            QRect t_pixMapRect(rect.x() + 150, rect.y() + 150, 40, 40);
            painter.drawPixmap(t_pixMapRect, t_hoverPlayImg);
        }
    }
    painter.restore();
    painter.fillRect(option.rect, t_fillBrush);
}

void SingerDataDelegate::touchClicked(const QStyleOptionViewItem &option, const QModelIndex &index, const QPointF pressPos)
{
    const SingerListView *singerListView = qobject_cast<const SingerListView *>(option.widget);
    SingerInfo signerTmp = index.data(Qt::UserRole).value<SingerInfo>();

    QRect rect = option.rect;

    QPainterPath imageClipPath;
    imageClipPath.addEllipse(QRect(rect.x() + 150, rect.y() + 150, 40, 40));
    imageClipPath.closeSubpath();
    auto fillPolygon = imageClipPath.toFillPolygon();

    if (fillPolygon.containsPoint(pressPos, Qt::OddEvenFill)) {
        bool playFlag = signerTmp.musicinfos.keys().contains(Player::getInstance()->getActiveMeta().hash);
        Player::PlaybackStatus playStatue = Player::getInstance()->status();
        if (playFlag) {
            if (playStatue == Player::Playing) {
                Player::getInstance()->pause();
            } else if (playStatue == Player::Paused) {
                Player::getInstance()->resume();
            }
        } else if (signerTmp.musicinfos.values().size() > 0) {
            emit CommonService::getInstance()->signalSetPlayModel(Player::RepeatAll);
            Player::getInstance()->setCurrentPlayListHash(singerListView->getHash(), false);
            Player::getInstance()->setPlayList(signerTmp.musicinfos.values());
            Player::getInstance()->playMeta(signerTmp.musicinfos.values().first());
            emit Player::getInstance()->signalPlayListChanged();
        }
    } else if (singerListView->getHash() == "artist") {
        // 切换到歌手二级页面
        emit CommonService::getInstance()->signalSwitchToView(SingerSubSongListType, "", signerTmp.musicinfos);
    } else if (singerListView->getHash() == "artistResult") {
        // 切换到搜索结果歌手二级页面
        emit CommonService::getInstance()->signalSwitchToView(SearchSingerSubSongListType, "", signerTmp.musicinfos);
    }
}

void SingerDataDelegate::touchDoubleClicked(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    const SingerListView *singerListView = qobject_cast<const SingerListView *>(option.widget);
    SingerInfo singertmp = index.data(Qt::UserRole).value<SingerInfo>();
    bool playFlag = singertmp.musicinfos.keys().contains(Player::getInstance()->getActiveMeta().hash);
    Player::PlaybackStatus playStatue = Player::getInstance()->status();
    if (playFlag) {
        if (playStatue == Player::Playing) {
            Player::getInstance()->pause();
        } else if (playStatue == Player::Paused) {
            Player::getInstance()->resume();
        }
    } else {
        if (singertmp.musicinfos.values().size() > 0) {
            emit CommonService::getInstance()->signalSetPlayModel(Player::RepeatAll);
            Player::getInstance()->setCurrentPlayListHash(singerListView->getHash(), false);
            Player::getInstance()->setPlayList(singertmp.musicinfos.values());
            Player::getInstance()->playMeta(singertmp.musicinfos.values().first());
            emit Player::getInstance()->signalPlayListChanged();
        }
    }
}

void SingerDataDelegate::drawListMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const SingerListView *listview = qobject_cast<const SingerListView *>(option.widget);
    SingerInfo singertmp = index.data(Qt::UserRole).value<SingerInfo>();

    QFont fontT9 = DFontSizeManager::instance()->get(DFontSizeManager::T9);
    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);

    QColor nameColor("#090909");
    QColor otherColor("#000000");

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    QColor baseColor("#FFFFFF");
    QColor alternateBaseColor("#000000");
    QColor selecteColor("#000000");
    baseColor.setAlphaF(0.1);
    alternateBaseColor.setAlphaF(0.02);
    selecteColor.setAlphaF(0.10);
    if (listview->getThemeType() == 2) {
        baseColor.setAlphaF(0.05);
        alternateBaseColor.setAlphaF(0.05);
        selecteColor = QColor("#FFFFFF");
        selecteColor.setAlphaF(0.20);
    }

    QColor background = (index.row() % 2) == 1 ? baseColor : alternateBaseColor;
    int lrWidth = 10;
    if (!(option.state & QStyle::State_Selected) && !(option.state & QStyle::State_MouseOver)) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(background);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
        painter.drawRoundedRect(selecteColorRect, 8, 8);
        painter.restore();
    }

    if (option.state & QStyle::State_Selected) {
        painter.save();
        painter.setPen(Qt::NoPen);
        QColor selectColor(option.palette.highlight().color());
        painter.setBrush(selectColor);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
        painter.drawRoundedRect(selecteColorRect, 8, 8);
        painter.restore();
    }

    if (option.state & QStyle::State_MouseOver) {
        painter.save();
        painter.setPen(Qt::NoPen);
        QColor hovertColor;
        if (listview->getThemeType() == 1) {
            hovertColor = option.palette.shadow().color();
        } else {
            hovertColor = QColor("#ffffff");
            hovertColor.setAlphaF(0.1);
        }
        if (option.state & QStyle::State_Selected)
            hovertColor.setAlphaF(0.2);
        painter.setBrush(hovertColor);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
        painter.drawRoundedRect(selecteColorRect, 8, 8);
        painter.restore();
    }

    otherColor.setAlphaF(0.5);
    if (listview->getThemeType() == 2) {
        nameColor = QColor("#C0C6D4");
        otherColor = QColor("#C0C6D4");
        // 按设计修改透明度
        otherColor.setAlphaF(0.6);
    }

    int rowCount = listview->model()->rowCount();
    int rowCountSize = QString::number(rowCount).size();
    rowCountSize = qMax(rowCountSize, 2);

    int iconSize = 32;
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
        iconSize = 26;
    }
#endif

    QFontMetrics songsFm(fontT9);
    QRect numRect(lrWidth + 10, option.rect.y() + (option.rect.height() - iconSize ) / 2, iconSize, iconSize);
    QIcon icon = option.icon;
    QVariant value = index.data(Qt::DecorationRole);
    if (value.type() == QVariant::Icon) {
        icon = qvariant_cast<QIcon>(value);
    }
    painter.save();
    QPainterPath clipPath;
    clipPath.addEllipse(numRect.adjusted(4, 4, -4, -4));
    painter.setClipPath(clipPath);
    painter.drawPixmap(numRect, icon.pixmap(numRect.width(), numRect.width()));
    painter.restore();

    if (option.state & QStyle::State_Selected) {
        nameColor = option.palette.highlightedText().color();
        otherColor = option.palette.highlightedText().color();
    }

    // name
    painter.setPen(nameColor);
    QRect nameRect(60, option.rect.y(), option.rect.width() / 2 - 20, option.rect.height());
    painter.setFont(fontT6);
    auto nameText = songsFm.elidedText(singertmp.singerName.isEmpty() ? SingerListView::tr("Unknown artist") :
                                       singertmp.singerName, Qt::ElideMiddle, nameRect.width());
    painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);


    // songs
    painter.setPen(otherColor);
    int sortMetasSize = singertmp.musicinfos.keys().size();;
    QString infoStr;
    if (sortMetasSize == 0) {
        infoStr = QString("   ") + tr("No songs");
    } else if (sortMetasSize == 1) {
        infoStr = QString("   ") + tr("1 song");
    } else {
        infoStr = QString("   ") + tr("%1 songs").arg(sortMetasSize);
    }
    painter.save();
    painter.setFont(fontT9);
    QFontMetrics measuringFont(fontT9);
    int strwidth = pixel2point(measuringFont.width(infoStr));
    QString countText = songsFm.elidedText(infoStr, Qt::ElideMiddle, strwidth);
    // 歌曲数量右对齐
    painter.drawText(option.rect.adjusted(0, 0, -PlayItemRightMargin, 0), Qt::AlignRight | Qt::AlignVCenter, countText);
    painter.restore();
}

void SingerDataDelegate::mouseClicked(const QStyleOptionViewItem &option, const QModelIndex &index, const QPointF pressPos)
{
    SingerInfo singertmp = index.data(Qt::UserRole).value<SingerInfo>();
    bool playFlag = singertmp.musicinfos.keys().contains(Player::getInstance()->getActiveMeta().hash);
    Player::PlaybackStatus playStatue = Player::getInstance()->status();

    if (playFlag) {
        if (playStatue == Player::Playing) {
            Player::getInstance()->pause();
        } else if (playStatue == Player::Paused) {
            Player::getInstance()->resume();
        }
    } else {
        int borderWidth = 10;
        QRect rect = option.rect.adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth);

        QPainterPath imageClipPath;
        imageClipPath.addEllipse(QRect(rect.x() + (rect.width() - playPauseBtnSize()) / 2, rect.y() + heightTopMargin(),
                                       playPauseBtnSize(), playPauseBtnSize()));
        imageClipPath.closeSubpath();
        QPolygonF fillPolygon = imageClipPath.toFillPolygon();

        if (fillPolygon.containsPoint(pressPos, Qt::OddEvenFill)) {
            if (singertmp.musicinfos.values().size() > 0) {
                emit CommonService::getInstance()->signalSetPlayModel(Player::RepeatAll);
                Player::getInstance()->setCurrentPlayListHash("artist", false);
                Player::getInstance()->setPlayList(singertmp.musicinfos.values());
                Player::getInstance()->playMeta(singertmp.musicinfos.values().first());
                emit Player::getInstance()->signalPlayListChanged();
            }
        }
    }
}

void SingerDataDelegate::mouseDoubleClicked(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    const SingerListView *singerListView = qobject_cast<const SingerListView *>(option.widget);
    SingerInfo signerTmp = index.data(Qt::UserRole).value<SingerInfo>();

    if (singerListView->getHash() == "artist") {
        // 切换到歌手二级页面
        emit CommonService::getInstance()->signalSwitchToView(SingerSubSongListType, "", signerTmp.musicinfos);
    } else if (singerListView->getHash() == "artistResult") {
        // 切换到搜索结果歌手二级页面
        emit CommonService::getInstance()->signalSwitchToView(SearchSingerSubSongListType, "", signerTmp.musicinfos);
    }
}

SingerDataDelegate::SingerDataDelegate(QWidget *parent): QStyledItemDelegate(parent)
    , hoverPlayImg(DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/actions/play_hover_36px.svg"))
    , hoverSuspendImg(DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/actions/suspend_hover_36px.svg"))
    , shadowImg(DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/actions/shadow_176px.svg"))
{
}

SingerDataDelegate::~SingerDataDelegate()
{

}

void SingerDataDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
}

