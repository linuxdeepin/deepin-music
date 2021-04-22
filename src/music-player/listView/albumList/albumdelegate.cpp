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

#include "albumdelegate.h"
#include "albumlistview.h"
#include "singerlistview.h"
#include "core/medialibrary.h"
#include "global.h"
#include "player.h"


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

const int PlayItemRightMargin = 20;

static inline int pixel2point(int pixel)
{
    return pixel * 96 / 72;
}

void AlbumDataDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    auto listview = qobject_cast<const AlbumListView *>(option.widget);
//    QList<AlbumInfo> albumlist = listview->getAlbumListData();
//    if (index.row() >= albumlist.size()) {
//        return;
//    }

    if (CommonService::getInstance()->isTabletEnvironment()) {
        if (listview->viewMode() == QListView::IconMode) {
            drawTabletIconMode(*painter, option, index);
        } else {
            this->drawListMode(*painter, option, index);
        }
    } else {
        if (listview->viewMode() == QListView::IconMode) {
            this->drawIconMode(*painter, option, index);
        } else {
            this->drawListMode(*painter, option, index);
        }
    }
}

QSize AlbumDataDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    auto *listview = qobject_cast<const AlbumListView *>(option.widget);
    if (listview && listview->viewMode() == QListView::IconMode) {
        if (CommonService::getInstance()->isTabletEnvironment()) {
            return QSize(200, 200);
        } else {
            return QSize(150, 150);
        }
    } else {
        auto baseSize = QStyledItemDelegate::sizeHint(option, index);
        return QSize(baseSize.width(), 38);
    }
}

bool AlbumDataDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (CommonService::getInstance()->isTabletEnvironment()) {
        const AlbumListView *albumlistView = qobject_cast<const AlbumListView *>(option.widget);
        const QMouseEvent *pressEvent = static_cast<QMouseEvent *>(event);
        const QPointF pressPos = pressEvent->pos();

        if (index.isValid() && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)) {
            // 用于判断触屏点击状态
            static int clickedCount = 0;
            // 触屏点击次数
            clickedCount++;

            QTimer::singleShot(200, [ = ]() {
                if (clickedCount == 1) {
                    if (albumlistView->viewMode() == QListView::IconMode) {
                        // IconMode触屏单击
                        touchClicked(option, index, pressPos);
                    } else if (albumlistView->viewMode() == QListView::ListMode) {
                        // 同鼠标双击，进入二级菜单
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
        const AlbumListView *albumlistView = qobject_cast<const AlbumListView *>(option.widget);
        const QMouseEvent *pressEvent = static_cast<QMouseEvent *>(event);
        const QPointF pressPos = pressEvent->pos();

        if (index.isValid() && albumlistView->viewMode() == QListView::IconMode &&
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
        } else if (index.isValid() && albumlistView->viewMode() == QListView::ListMode && event->type() == QEvent::MouseButtonDblClick) {
            // 鼠标双击
            mouseDoubleClicked(option, index);
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void AlbumDataDelegate::drawIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto listview = qobject_cast<const AlbumListView *>(option.widget);
    AlbumInfo albumTmp = index.data(Qt::UserRole).value<AlbumInfo>();

    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    QFont fontT9 = DFontSizeManager::instance()->get(DFontSizeManager::T9);

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

    //draw border
    painter.save();
    QColor borderPenColor("#000000");
    borderPenColor.setAlphaF(0.05);
    QPen borderPen(borderPenColor);
    borderPen.setWidthF(2);
    painter.setPen(borderPen);
    painter.drawRoundRect(rect/*.adjusted(1, 1, -1, 1)*/, 10, 10);
    painter.restore();

    bool playFlag = albumTmp.musicinfos.keys().contains(Player::getInstance()->getActiveMeta().hash);
    Player::PlaybackStatus playStatue = Player::getInstance()->status();

    QColor fillColor(0, 0, 0);
    fillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        fillColor = "#000000";
        fillColor.setAlphaF(0.3);
    }
    int startHeight = rect.y() + rect.height() - 46;
    int fillAllHeight = 46;
    int curFillSize = fillAllHeight;
    QRect fillBlurRect(rect.x(), rect.y() + rect.height() - fillAllHeight, rect.width(), fillAllHeight);

    if (playFlag && (playStatue == Player::Playing)) {
        fillBlurRect = QRect(rect.x(), rect.y() + rect.height() - 80, rect.width(), 86);
        curFillSize = 80;
    }

    // 设置模糊
    QImage t_imageBlur = opticon.pixmap(rect.width(), rect.height()).toImage();
    qreal t_ratioBlur = t_imageBlur.devicePixelRatioF();
    curFillSize = static_cast<int>(curFillSize * t_ratioBlur);

    t_imageBlur  = t_imageBlur.copy(0, rect.height() - curFillSize, t_imageBlur.width(), curFillSize);
    QTransform old_transformBlur = painter.transform();
    painter.translate(fillBlurRect.topLeft());
    qt_blurImage(&painter, t_imageBlur, 35, false, false);
    painter.setTransform(old_transformBlur);
    // 设置模糊
    painter.fillRect(fillBlurRect, fillColor);

    // draw playing
    auto *listview2 = qobject_cast<AlbumListView *>(const_cast<QWidget *>(option.widget));
    if (playFlag && playStatue == Player::Playing) {
        if (option.state & QStyle::State_MouseOver) {
            painter.drawPixmap(QRect(rect.x() + 56, rect.y() + 72, 36, 36), hoverSuspendImg);
        } else {
            if (listview2) {
                painter.drawPixmap(QRect(rect.x() + 64, rect.y() + 82, 20, 18), listview2->getPlayPixmap(true));
            }
        }
    }

    QRect fillRect(rect.x(), startHeight, rect.width(), fillAllHeight);

    QFontMetrics nameTextFm(fontT6);
    painter.setFont(fontT6);
    QRect nameFillRect(rect.x(), startHeight + 2, rect.width(), fillAllHeight * 3 / 2);
    nameFillRect.adjust(8, 0, -7, 0);
    QString nameText = nameTextFm.elidedText(albumTmp.albumName.isEmpty() ? AlbumListView::tr("Unknown album") : albumTmp.albumName, Qt::ElideMiddle, nameFillRect.width());
    painter.setPen(Qt::white);
    painter.drawText(nameFillRect, Qt::AlignLeft | Qt::AlignTop, nameText);

    QFontMetrics extraNameFm(fontT9);
    painter.setFont(fontT9);
    QRect extraNameFillRect(rect.x(), startHeight + fillAllHeight / 2 + 1, rect.width(), fillAllHeight / 2);
    extraNameFillRect.adjust(8, 0, -7, 0);
    QString extraNameText = extraNameFm.elidedText(albumTmp.singer.isEmpty() ? SingerListView::tr("Unknown artist") : albumTmp.singer, Qt::ElideMiddle, extraNameFillRect.width());
    painter.setPen(Qt::white);
    painter.drawText(extraNameFillRect, Qt::AlignLeft | Qt::AlignTop, extraNameText);

    QBrush fillBrush(QColor(128, 128, 128, 0));

    fillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        fillColor = "#000000";
        fillColor.setAlphaF(0.3);
    }

    if (option.state & QStyle::State_Selected) {
        fillBrush = QBrush(QColor(128, 128, 128, 90));
    }

    if ((option.state & QStyle::State_MouseOver)  && (!playFlag || (playStatue == Player::Paused))) {
        QImage t_image = opticon.pixmap(rect.width(), rect.height()).toImage();
        int t_ratio = static_cast<int>(t_image.devicePixelRatioF());
        QRect t_imageRect(rect.width() / 2 - 25, rect.height() / 2 - 25, 60 * t_ratio, 60 * t_ratio);
        t_image  = t_image.copy(t_imageRect);
        QRect t_hoverRect(rect.x() + 50, rect.y() + 36, 50 * t_ratio, 50 * t_ratio);

        QTransform old_transform = painter.transform();
        painter.translate(t_hoverRect.topLeft());

        QPainterPath t_imageClipPath;
        t_imageClipPath.addEllipse(QRect(0, 0, 50, 50));
        painter.setClipPath(t_imageClipPath);

        qt_blurImage(&painter, t_image, 30, false, false);
        painter.setTransform(old_transform);
        painter.fillRect(t_hoverRect, fillColor);

        QPixmap t_hoverPlayImg(hoverPlayImg);
        t_hoverPlayImg.setDevicePixelRatio(option.widget->devicePixelRatioF());
        //            t_hoverRect.adjust(0, 0, -7 * t_ratio, -7 * t_ratio);
        QRect t_pixMapRect(rect.x() + 53, rect.y() + 40, 43, 43);
        painter.drawPixmap(t_pixMapRect, t_hoverPlayImg);
    }

    painter.fillRect(option.rect, fillBrush);
}

QPixmap blurPixmap(const QPixmap &pix, int radius, int tp, const QRect &clipRect)
{
    QPixmap tmpPixmap = pix;

    if (clipRect.isValid()) {
        tmpPixmap = tmpPixmap.copy(clipRect);
    }

    int imgWidth = tmpPixmap.width();
    int imgHeigth = tmpPixmap.height();
    if (!tmpPixmap.isNull()) {
        tmpPixmap = tmpPixmap.scaled(imgWidth / radius, imgHeigth / radius, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if (tp == 0) {
            tmpPixmap = tmpPixmap.scaled(imgWidth, imgHeigth, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        } else {
            tmpPixmap = tmpPixmap.scaled(imgWidth, imgHeigth);
        }
    }
    return tmpPixmap;
}

void AlbumDataDelegate::drawTabletIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto listview = qobject_cast<const AlbumListView *>(option.widget);
    AlbumInfo albumTmp = index.data(Qt::UserRole).value<AlbumInfo>();

    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    QFont fontT9 = DFontSizeManager::instance()->get(DFontSizeManager::T9);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    auto background = option.palette.background();
    painter.fillRect(option.rect, background);

    // 绘制阴影
    QRect shadowRect(option.rect.x(), option.rect.y(), 200, 200);
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

    //draw border
    painter.save();
    QColor borderPenColor("#000000");
    borderPenColor.setAlphaF(0.05);
    QPen borderPen(borderPenColor);
    borderPen.setWidthF(2);
    painter.setPen(borderPen);
    painter.drawRoundRect(rect/*.adjusted(1, 1, -1, 1)*/, 10, 10);
    painter.restore();

    bool playFlag = albumTmp.musicinfos.keys().contains(Player::getInstance()->getActiveMeta().hash);
    Player::PlaybackStatus playStatue = Player::getInstance()->status();

    QColor fillColor(0, 0, 0);
    fillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        fillColor = "#000000";
        fillColor.setAlphaF(0.3);
    }
    int startHeight = rect.y() + rect.height() - 46;
    int fillAllHeight = 57;
    QRect fillBlurRect(rect.x(), rect.y() + rect.height() - fillAllHeight, rect.width(), fillAllHeight);

    QPixmap cov;
    cov = opticon.pixmap(200, 200);
    cov = cov.scaled(200, 200);
    QRect target;
    auto *listview2 = qobject_cast<AlbumListView *>(const_cast<QWidget *>(option.widget));
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
// 专辑名称
    QFontMetrics nameTextFm(fontT6);
    painter.setFont(fontT6);
    QRect nameFillRect(option.rect.x() + 8, option.rect.y() + 150, 135, 40);
    QString nameText = nameTextFm.elidedText(albumTmp.albumName.isEmpty() ? AlbumListView::tr("Unknown album") : albumTmp.albumName, Qt::ElideMiddle, nameFillRect.width());
    painter.setPen(Qt::white);
    painter.drawText(nameFillRect, Qt::AlignLeft | Qt::AlignTop, nameText);
// 歌手名称
    QFontMetrics singerNameFm(fontT9);
    painter.setFont(fontT9);
    QRect singerNameFillRect(option.rect.x() + 8, option.rect.y() + 173, 135, 40);
    QString singerNameText = singerNameFm.elidedText(albumTmp.singer.isEmpty() ? SingerListView::tr("Unknown artist") : albumTmp.singer, Qt::ElideMiddle, singerNameFillRect.width());
    painter.setPen(Qt::white);
    painter.drawText(singerNameFillRect, Qt::AlignLeft | Qt::AlignTop, singerNameText);

    QBrush fillBrush(QColor(128, 128, 128, 0));

    fillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        fillColor = "#000000";
        fillColor.setAlphaF(0.3);
    }

    if (option.state & QStyle::State_Selected) {
        fillBrush = QBrush(QColor(128, 128, 128, 90));
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
    painter.fillRect(option.rect, fillBrush);
}

void AlbumDataDelegate::drawListMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto listview = qobject_cast<const AlbumListView *>(option.widget);
    AlbumInfo albumTmp = index.data(Qt::UserRole).value<AlbumInfo>();

    QFont fontT9 = DFontSizeManager::instance()->get(DFontSizeManager::T9);
    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);

    QColor nameColor("#090909");
    QColor otherColor("#000000");

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    QColor baseColor("#FFFFFF");
    baseColor.setAlphaF(0.1);
    QColor alternateBaseColor("#000000");
    alternateBaseColor.setAlphaF(0.02);
    QColor selecteColor("#000000");
    selecteColor.setAlphaF(0.10);
    if (listview->getThemeType() == 2) {
        baseColor.setAlphaF(0.05);
        alternateBaseColor.setAlphaF(0.05);
        selecteColor = QColor("#FFFFFF");
        selecteColor.setAlphaF(0.20);
    }

    auto background = (index.row() % 2) == 1 ? baseColor : alternateBaseColor;

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
    auto rowCountSize = QString::number(rowCount).size();
    rowCountSize = qMax(rowCountSize, 2);

    QFontMetrics songsFm(fontT9);
    auto tailwidth = pixel2point(songsFm.width("0000-00-00")) + PlayItemRightMargin  + 20;
    auto w = option.rect.width() - 0 - tailwidth;

    auto *listview2 = qobject_cast<AlbumListView *>(const_cast<QWidget *>(option.widget));
    bool playFlag = albumTmp.musicinfos.keys().contains(Player::getInstance()->getActiveMeta().hash);
    //num
    if (playFlag) {
        QPixmap playicon;
        if (option.state & QStyle::State_Selected) {
            nameColor = option.palette.highlightedText().color();
            otherColor = option.palette.highlightedText().color();
        } else {
            nameColor = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
            // 未选中时，显示活动色
            otherColor = nameColor;
        }
        QRect numRect(lrWidth, option.rect.y(), 40, option.rect.height());
        if (option.state & QStyle::State_Selected) {
            if (listview2) {
                playicon = listview2->getPlayPixmap(true);
            }
        } else {
            if (listview2) {
                playicon = listview2->getPlayPixmap(false);
            }
        }
        qreal t_ratio = playicon.devicePixelRatioF();
        auto centerF = numRect.center();
        QRect t_ratioRect;
        t_ratioRect.setX(0);
        t_ratioRect.setY(0);
        t_ratioRect.setWidth(static_cast<int>(playicon.width() / t_ratio));
        t_ratioRect.setHeight(static_cast<int>(playicon.height() / t_ratio));
        auto iconRect = QRectF(centerF.x() - t_ratioRect.width() / 2,
                               centerF.y() - t_ratioRect.height() / 2,
                               t_ratioRect.width(), t_ratioRect.height());
        painter.drawPixmap(iconRect.toRect(), playicon);
    } else {
        if (option.state & QStyle::State_Selected) {
            nameColor = option.palette.highlightedText().color();
            otherColor = option.palette.highlightedText().color();
        }

        painter.setPen(otherColor);
        QRect numRect(lrWidth, option.rect.y(), 40, option.rect.height());
        painter.setFont(fontT9);
        // 序号显示由001改为1
        QString str = QString::number(index.row() + 1);
        QFontMetrics fm(fontT9);
        auto text = fm.elidedText(str, Qt::ElideMiddle, numRect.width());
        painter.drawText(numRect, Qt::AlignCenter, text);
    }

    // name
    painter.setPen(nameColor);
    QRect nameRect(50, option.rect.y(), w / 2 - 20, option.rect.height());
    painter.setFont(fontT6);
    // 解决文字被截断问题
    auto nameText = songsFm.elidedText(albumTmp.albumName.isEmpty() ? AlbumListView::tr("Unknown album") : albumTmp.albumName, Qt::ElideMiddle, nameRect.width() - 70);
    painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

    // extraname
    // 按设计修改透明度
    painter.setPen(otherColor);
    QRect extraRect(50 + w / 2, option.rect.y(), w / 4 - 20, option.rect.height());
    painter.setFont(fontT9);
    auto extraText = songsFm.elidedText(albumTmp.singer.isEmpty() ? SingerListView::tr("Unknown artist") : albumTmp.singer, Qt::ElideMiddle, extraRect.width());
    painter.drawText(extraRect, Qt::AlignLeft | Qt::AlignVCenter, extraText);

    // songs
    int sortMetasSize = albumTmp.musicinfos.size();
    QString infoStr;
    if (sortMetasSize == 0) {
        infoStr = QString("   ") + tr("No songs");
    } else if (sortMetasSize == 1) {
        infoStr = QString("   ") + tr("1 song");
    } else {
        infoStr = QString("   ") + tr("%1 songs").arg(sortMetasSize);
    }

    QFont measuringFont(fontT9);
    QRect songsRect(50 + w / 2 + w / 4, option.rect.y(), w / 4 - 20, option.rect.height());
    painter.drawText(songsRect, Qt::AlignLeft | Qt::AlignVCenter, infoStr);

    // day
    QRect dayRect(w, option.rect.y(), tailwidth - 20, option.rect.height());
    painter.setFont(fontT9);
    QString dayStr = QDateTime::fromMSecsSinceEpoch(albumTmp.timestamp / static_cast<qint64>(1000)).toString("yyyy-MM-dd");
    painter.drawText(dayRect, Qt::AlignRight | Qt::AlignVCenter, dayStr);

    painter.restore();
}

void AlbumDataDelegate::mouseClicked(const QStyleOptionViewItem &option, const QModelIndex &index, const QPointF pressPos)
{
    AlbumInfo albumTmp = index.data(Qt::UserRole).value<AlbumInfo>();
    bool playFlag = albumTmp.musicinfos.keys().contains(Player::getInstance()->getActiveMeta().hash);
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
        QRect hoverRect(rect.x() + 50, rect.y() + 36, 50, 50);

        QPainterPath imageClipPath;
        imageClipPath.addEllipse(QRect(rect.x() + 50, rect.y() + 36, 50, 50));
        imageClipPath.closeSubpath();
        auto fillPolygon = imageClipPath.toFillPolygon();

        if (fillPolygon.containsPoint(pressPos, Qt::OddEvenFill)) {
            if (albumTmp.musicinfos.values().size() > 0) {
                emit CommonService::getInstance()->signalSetPlayModel(Player::RepeatAll);
                Player::getInstance()->setCurrentPlayListHash("album", false);
                Player::getInstance()->setPlayList(albumTmp.musicinfos.values());
                Player::getInstance()->playMeta(albumTmp.musicinfos.values().first());
                emit Player::getInstance()->signalPlayListChanged();
            }
        }
    }
}

void AlbumDataDelegate::mouseDoubleClicked(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    const AlbumListView *albumlistView = qobject_cast<const AlbumListView *>(option.widget);
    AlbumInfo albumTmp = index.data(Qt::UserRole).value<AlbumInfo>();

    if (albumlistView->getHash() == "album") {
        // 切换到专辑二级页面
        emit CommonService::getInstance()->signalSwitchToView(AlbumSubSongListType, "", albumTmp.musicinfos);
    } else if (albumlistView->getHash() == "albumResult") {
        // 切换到搜索结果专辑二级页面
        emit CommonService::getInstance()->signalSwitchToView(SearchAlbumSubSongListType, "", albumTmp.musicinfos);
    }
}

void AlbumDataDelegate::touchClicked(const QStyleOptionViewItem &option, const QModelIndex &index, const QPointF pressPos)
{
    const AlbumListView *albumlistView = qobject_cast<const AlbumListView *>(option.widget);
    AlbumInfo albumTmp = index.data(Qt::UserRole).value<AlbumInfo>();

    QRect rect = option.rect;

    QPainterPath imageClipPath;
    imageClipPath.addEllipse(QRect(rect.x() + 150, rect.y() + 150, 40, 40));
    imageClipPath.closeSubpath();
    auto fillPolygon = imageClipPath.toFillPolygon();

    if (fillPolygon.containsPoint(pressPos, Qt::OddEvenFill)) {
        bool playFlag = albumTmp.musicinfos.keys().contains(Player::getInstance()->getActiveMeta().hash);
        Player::PlaybackStatus playStatue = Player::getInstance()->status();
        if (playFlag) {
            if (playStatue == Player::Playing) {
                Player::getInstance()->pause();
            } else if (playStatue == Player::Paused) {
                Player::getInstance()->resume();
            }
        } else if (albumTmp.musicinfos.values().size() > 0) {
            emit CommonService::getInstance()->signalSetPlayModel(Player::RepeatAll);
            Player::getInstance()->setCurrentPlayListHash(albumlistView->getHash(), false);
            Player::getInstance()->setPlayList(albumTmp.musicinfos.values());
            Player::getInstance()->playMeta(albumTmp.musicinfos.values().first());
            emit Player::getInstance()->signalPlayListChanged();
        }
    } else if (albumlistView->getHash() == "album") {
        // 切换到专辑二级页面
        emit CommonService::getInstance()->signalSwitchToView(AlbumSubSongListType, "", albumTmp.musicinfos);
    } else if (albumlistView->getHash() == "albumResult") {
        // 切换到搜索结果专辑二级页面
        emit CommonService::getInstance()->signalSwitchToView(SearchAlbumSubSongListType, "", albumTmp.musicinfos);
    }
}

void AlbumDataDelegate::touchDoubleClicked(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    const AlbumListView *albumlistView = qobject_cast<const AlbumListView *>(option.widget);
    AlbumInfo albumTmp = index.data(Qt::UserRole).value<AlbumInfo>();
    bool playFlag = albumTmp.musicinfos.keys().contains(Player::getInstance()->getActiveMeta().hash);
    Player::PlaybackStatus playStatue = Player::getInstance()->status();
    if (playFlag) {
        if (playStatue == Player::Playing) {
            Player::getInstance()->pause();
        } else if (playStatue == Player::Paused) {
            Player::getInstance()->resume();
        }
    } else {
        if (albumTmp.musicinfos.values().size() > 0) {
            emit CommonService::getInstance()->signalSetPlayModel(Player::RepeatAll);
            Player::getInstance()->setCurrentPlayListHash(albumlistView->getHash(), false);
            Player::getInstance()->setPlayList(albumTmp.musicinfos.values());
            Player::getInstance()->playMeta(albumTmp.musicinfos.values().first());
            emit Player::getInstance()->signalPlayListChanged();
        }
    }
}

AlbumDataDelegate::AlbumDataDelegate(QWidget *parent): QStyledItemDelegate(parent)
    , hoverPlayImg(DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/actions/play_hover_36px.svg"))
    , hoverSuspendImg(DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/actions/suspend_hover_36px.svg"))
    , shadowImg(DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/actions/shadow_176px.svg"))
{
    shadowImg = shadowImg.copy(5, 5, shadowImg.width() - 10, shadowImg.height() - 10);
}

AlbumDataDelegate::~AlbumDataDelegate()
{

}

void AlbumDataDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
}

