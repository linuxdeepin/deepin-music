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

#include "playitemdelegate.h"

#include <QDebug>
#include <QFont>
#include <QPainter>
#include <QFileInfo>
#include <QPainterPath>
#include <QStandardItemModel>
#include <QMouseEvent>

#include <DGuiApplicationHelper>
#include <DHiDPIHelper>
#include <DStyle>

#include "playlistview.h"
#include "databaseservice.h"
#include "player.h"

DWIDGET_USE_NAMESPACE

const int PlayItemRightMargin = 20;
const int ImgWidthAndHeight = 150;
const int xoffset = 10;
const int yoffset = 8;
const int roundRadius = 10;

// 平板模式
const int ImgWidthAndHeightTablet = 195;
const int yoffsetTablet = 5;
const int roundRadiusTablet = 10;

static inline int pixel2point(int pixel)
{
    return pixel * 96 / 72;
}

inline int tailPointWidth(const QStyleOptionViewItem &option)
{
    QFont measuringFont(option.font);
    QFontMetrics fm(measuringFont);
    return pixel2point(fm.width("00:00")) + PlayItemRightMargin;
}

static inline QFlags<Qt::AlignmentFlag> alignmentFlag(int col)
{
    auto emCol = static_cast<PlayItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case PlayItemDelegate::Number:
        return Qt::AlignCenter;
    case PlayItemDelegate::Title:
    case PlayItemDelegate::Artist:
    case PlayItemDelegate::Album:
        return (Qt::AlignLeft | Qt::AlignVCenter);
    case PlayItemDelegate::Length:
        return (Qt::AlignRight | Qt::AlignVCenter);
    case PlayItemDelegate::ColumnButt:
        break;
    }
    return (Qt::AlignLeft | Qt::AlignVCenter);;
}

static inline QRect colRect(int col, const QStyleOptionViewItem &option)
{
    static auto tailwidth  = tailPointWidth(option) + 20;
    auto w = option.rect.width() - 0 - tailwidth;

    auto emCol = static_cast<PlayItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case PlayItemDelegate::Number:
        return QRect(10, option.rect.y(), 40, option.rect.height());
    case PlayItemDelegate::Title:
        return QRect(50, option.rect.y(), w / 2 - 20, option.rect.height());
    case PlayItemDelegate::Artist:
        return QRect(50 + w / 2, option.rect.y(), w / 4 - 20, option.rect.height());
    case PlayItemDelegate::Album:
        return QRect(50 + w / 2 + w / 4, option.rect.y(), w / 4 - 20, option.rect.height());
    case PlayItemDelegate::Length:
        return QRect(w, option.rect.y(), tailwidth - 20, option.rect.height());
    case PlayItemDelegate::ColumnButt:
        break;
    }
    return option.rect.marginsRemoved(QMargins(0, 0, 0, 0));
}

void PlayItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    auto listview = qobject_cast<const PlayListView *>(option.widget);
    if (listview->viewMode() == QListView::IconMode) {
        if (CommonService::getInstance()->isTabletEnvironment()) {
            drawTabletIconMode(*painter, option, index);
        } else {
            drawIconMode(*painter, option, index);
        }
    } else {
        if (CommonService::getInstance()->isTabletEnvironment()) {
            drawTabletListMode(*painter, option, index);
        } else {
            drawListMode(*painter, option, index);
        }
    }
}

QSize PlayItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    auto *listview = qobject_cast<const PlayListView *>(option.widget);
    if (listview && listview->viewMode() == QListView::IconMode) {
        if (CommonService::getInstance()->isTabletEnvironment()) {
            return QSize(200, 243);
        } else {
            // 调整Icon间距
            return QSize(170, 210);
        }
    } else {
        auto baseSize = QStyledItemDelegate::sizeHint(option, index);
        return QSize(baseSize.width(), 38);
    }
}

bool PlayItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    const PlayListView *listview = qobject_cast<const PlayListView *>(option.widget);
    const QMouseEvent *pressEvent = static_cast<QMouseEvent *>(event);
    const QPointF pressPos = pressEvent->pos();

    if (index.isValid() && listview->viewMode() == QListView::IconMode) {
        if (pressPos.x() > (option.rect.x() + 160) // 点在图片右边
                || pressPos.y() > (option.rect.y() + option.rect.height() - 12) // 点在文字下面
                || pressPos.y() < (option.rect.y() + 8) // 点在图片上面
                || pressPos.x() < (option.rect.x() + 10)) { // 点在图片左边
            // 点击在空白处，清空选中
            const_cast<PlayListView *>(listview)->clearSelection();
            return true;
        } else {
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        }
    } else {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }
}

void PlayItemDelegate::drawTabletIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const PlayListView *listview = qobject_cast<const PlayListView *>(option.widget);

    QFont fontT9 = DFontSizeManager::instance()->get(DFontSizeManager::T9);
    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);

    MediaMeta activeMeta = Player::getInstance()->getActiveMeta();
    MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 绘制专辑图片
    painter.save();
    QIcon icon;
    auto value = index.data(Qt::DecorationRole);
    if (value.type() == QVariant::Icon) {
        icon = qvariant_cast<QIcon>(value);
    }
    QRect pixmapRect(option.rect.x(), option.rect.y() + 5, ImgWidthAndHeightTablet, ImgWidthAndHeightTablet);
    QPainterPath roundPixmapRectPath;
    roundPixmapRectPath.addRoundRect(pixmapRect, roundRadiusTablet, roundRadiusTablet);
    painter.setClipPath(roundPixmapRectPath);
    painter.drawPixmap(pixmapRect, icon.pixmap(ImgWidthAndHeightTablet, ImgWidthAndHeightTablet));
    painter.restore();
    // 绘制图片上添加描边
    painter.save();
    QColor borderPenColor("#000000");
    borderPenColor.setAlphaF(0.05);
    QPen borderPen(borderPenColor);
    borderPen.setWidthF(2);
    painter.setPen(borderPen);
    painter.drawRoundRect(pixmapRect/*.adjusted(1, 1, -1, 1)*/, roundRadius, roundRadius);
    painter.restore();

//    int startHeight = option.rect.y() + 159;
//    int fillAllHeight = 34;

    // 设置信息字体大小
    painter.setFont(fontT6);
    QFontMetrics fm(fontT6);
    QColor nameColor = "#000000";
    if (listview->getThemeType() == 2) {
        nameColor = "#C0C6D4";
    }
    painter.setPen(nameColor);
    // 调整歌曲名位置
    QRect nameFillRect(option.rect.x(), option.rect.y() + yoffsetTablet + ImgWidthAndHeightTablet,
                       ImgWidthAndHeightTablet, fm.height());
    auto nameText = fm.elidedText(meta.title, Qt::ElideRight, ImgWidthAndHeightTablet - 60);
    painter.drawText(nameFillRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

    QFontMetrics extraNameFm(fontT9);
    painter.setFont(fontT9);
    nameColor.setAlphaF(0.5);
    painter.setPen(nameColor);
    // 调整歌手位置
    QRect extraNameFillRect(option.rect.x(), nameFillRect.bottom(),
                            ImgWidthAndHeightTablet, extraNameFm.height());
    auto extraNameText = extraNameFm.elidedText(meta.singer, Qt::ElideRight, ImgWidthAndHeightTablet - 60);
    painter.drawText(extraNameFillRect, Qt::AlignLeft | Qt::AlignVCenter, extraNameText);

    // 画时间矩形
    QString timeText = DMusic::lengthString(meta.length);
    // 如果时间超过一小时，矩形绘制长一点
    QRect timeFillRect;
    if (timeText.size() > 5) {
        timeFillRect = QRect(option.rect.x() + ImgWidthAndHeightTablet - 58,
                             extraNameFillRect.top() + extraNameFillRect.height() / 2 - 8,
                             48, 16);
    } else {
        timeFillRect = QRect(option.rect.x() + ImgWidthAndHeightTablet - 48,
                             extraNameFillRect.top() + extraNameFillRect.height() / 2 - 8,
                             38, 16);
    }
    painter.save();
    QColor timeFillColor("#232323");
    timeFillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        timeFillColor = "#DCDCDC";
        timeFillColor.setAlphaF(0.3);
    }
    painter.setPen(Qt::NoPen);
    painter.setBrush(timeFillColor);
    painter.drawRoundedRect(timeFillRect, 8, 8);
    painter.restore();

    // 时间字体固定大小
    fontT9.setPixelSize(11);
    painter.setFont(fontT9);

    // 画时间
    QColor timedColor = Qt::white;
    if (listview->getThemeType() == 2) {
        timedColor = "#C0C6D4";
    }
    painter.setPen(timedColor);
    painter.drawText(timeFillRect, Qt::AlignCenter, timeText);

    // 绘制选中时的阴影
    QBrush fillBrush(QColor(128, 128, 128, 0));
    QPixmap scacheicon = m_unselectedPix;
    if (meta.beSelect) {
        fillBrush = QBrush(QColor(128, 128, 128, 90));
        scacheicon = m_selectedPix;
    }
    painter.save();
    painter.setClipPath(roundPixmapRectPath);
    painter.fillRect(pixmapRect, fillBrush);
    painter.restore();
    // 绘制选中时右上角的选中图标
    if (option.state && (CommonService::getInstance()->getSelectModel() == CommonService::MultSelect)) {
        QRect selectionRect(option.rect.x() +  option.rect.width() - 18, option.rect.y() + 2, 14, 14);
        painter.drawPixmap(selectionRect, scacheicon);
    }
}

void PlayItemDelegate::drawTabletListMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const PlayListView *listview = qobject_cast<const PlayListView *>(option.widget);
//    if (listview->getIsPlayQueue()) {
//        drawListMode(painter, option, index);
//        return;
//    }

    QFont fontT9 = DFontSizeManager::instance()->get(DFontSizeManager::T9);
    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);

    QColor nameColor("#090909");
    QColor otherColor("#000000");

    painter.save();

    CommonService::TabletSelectMode selectMod = CommonService::getInstance()->getSelectModel();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QColor baseColor("#FFFFFF");
    baseColor.setAlphaF(0.1);
    QColor alternateBaseColor("#000000");
    alternateBaseColor.setAlphaF(0.02);
    QColor selecteColor("#000000");
    selecteColor.setAlphaF(0.20);
    if (listview->getThemeType() == 2) {
        baseColor.setAlphaF(0.05);
        alternateBaseColor.setAlphaF(0.05);
        selecteColor = QColor("#FFFFFF");
        selecteColor.setAlphaF(0.20);
    }
    auto background = (index.row() % 2) == 1 ? baseColor : alternateBaseColor;
    int lrWidth = 10;
    // 多选模式阴影向右偏移
    if (!listview->getIsPlayQueue() && selectMod == CommonService::MultSelect) {
        lrWidth = 40;
    }
    // 当前绘制项
    MediaMeta itemMeta = index.data(Qt::UserRole).value<MediaMeta>();
    if (!itemMeta.beSelect && !(option.state & QStyle::State_MouseOver)) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(background);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
        painter.drawRoundedRect(selecteColorRect, 8, 8);
        painter.restore();
    }

    otherColor.setAlphaF(0.5);
    if (listview->getThemeType() == 2) {
        nameColor = QColor("#C0C6D4");
        otherColor = QColor("#C0C6D4");
        otherColor.setAlphaF(0.6);
    }

    QPixmap scacheicon = m_unselectedPix;
    MediaMeta activeMeta = Player::getInstance()->getActiveMeta();
    if (activeMeta.hash == itemMeta.hash) {
        nameColor = QColor(DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
        otherColor = QColor(DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
        fontT6.setFamily("SourceHanSansSC");
        fontT6.setWeight(QFont::Medium);
    }

    if (itemMeta.beSelect/*option.state & QStyle::State_Selected*/) {

        if (selectMod == CommonService::SingleSelect || listview->getIsPlayQueue()) {
            painter.save();
            painter.setPen(Qt::NoPen);
            QColor selectColor(option.palette.highlight().color());
            painter.setBrush(selectColor);
            QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
            painter.drawRoundedRect(selecteColorRect, 8, 8);
            painter.restore();

            nameColor = option.palette.highlightedText().color();
            otherColor = option.palette.highlightedText().color();
        }

        scacheicon = m_selectedPix;
    }
    // 平板没有hover状态
//    if (option.state & QStyle::State_MouseOver) {
//        painter.save();
//        painter.setPen(Qt::NoPen);
//        QColor hovertColor;
//        if (listview->getThemeType() == 1) {
//            hovertColor = option.palette.shadow().color();
//        } else {
//            hovertColor = QColor("#ffffff");
//            hovertColor.setAlphaF(0.1);
//        }
//        if (option.state & QStyle::State_Selected)
//            hovertColor.setAlphaF(0.2);
//        painter.setBrush(hovertColor);
//        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
//        painter.drawRoundedRect(selecteColorRect, 8, 8);
//        painter.restore();
//    }

    int offset = 10;
    for (int col = 0; col < ColumnButt; ++col) {
        auto flag = alignmentFlag(col);
        auto rect = colRect(col, option);
        switch (col) {
        case Number: {
            painter.setPen(otherColor);
            PlayListView *playListView = qobject_cast<PlayListView *>(const_cast<QWidget *>(option.widget));

            if (!listview->getIsPlayQueue() && selectMod == CommonService::MultSelect) {
                offset = 35;
                auto sz = QSizeF(14, 14);
                auto iconRect = QRectF(option.rect.x() + 8,
                                       option.rect.y() + option.rect.height() / 2 - sz.height() / 2,
                                       sz.width(), sz.height());
                painter.drawPixmap(iconRect, scacheicon, QRectF());
            }
            // Fixme:
            QFileInfo info(itemMeta.localPath);
            if (!info.exists() && itemMeta.mmType != MIMETYPE_CDA) {
                auto sz = QSizeF(20, 20);
                auto icon = QIcon::fromTheme("icon_warning").pixmap(sz.toSize());
                auto centerF = QRectF(rect).center();
                auto iconRect = QRectF(centerF.x() - sz.width() / 2 + offset,
                                       centerF.y() - sz.height() / 2,
                                       sz.width(), sz.height());
                painter.drawPixmap(iconRect, icon, QRectF());
                break;
            }

            //绘制播放动态图
            if (activeMeta.hash == itemMeta.hash) {
                QPixmap icon;
                if (selectMod == CommonService::MultSelect) {
                    icon = playListView->getPlayPixmap(listview->getIsPlayQueue() ? itemMeta.beSelect : false);
                } else {
                    icon = playListView->getPlayPixmap(itemMeta.beSelect);
                }
                auto centerF = QRectF(rect).center();
                qreal t_ratio = icon.devicePixelRatioF();
                QRect t_ratioRect;
                t_ratioRect.setX(0);
                t_ratioRect.setY(0);
                t_ratioRect.setWidth(static_cast<int>(icon.width() / t_ratio));
                t_ratioRect.setHeight(static_cast<int>(icon.height() / t_ratio));
                auto iconRect = QRectF(centerF.x() - t_ratioRect.width() / 2 + offset - 10,
                                       centerF.y() - t_ratioRect.height() / 2,
                                       t_ratioRect.width(), t_ratioRect.height());
                painter.drawPixmap(iconRect.toRect(), icon);
            } else {
                painter.setFont(fontT9);
                // 只显示行号，如总数100,原来显示001修改为显示1
                auto str = QString::number(index.row() + 1);
                QFont font(fontT9);
                QFontMetrics fm(font);
                auto text = fm.elidedText(str, Qt::ElideLeft, rect.width());
                QRect r0 = rect;
                r0.setX(option.rect.x() + offset);
                r0.setWidth(40);
                painter.drawText(r0, static_cast<int>(flag), text);
            }
            break;
        }
        case Title: {
            painter.setPen(nameColor);
            painter.setFont(fontT6);
            QFont font(fontT6);
            QFontMetrics fm(font);
            auto text = fm.elidedText(itemMeta.title, Qt::ElideLeft, rect.width());
            QRect r1 = rect;
            r1.setX(rect.x() + offset - 10);
            r1.setWidth(rect.width());
            painter.drawText(r1, static_cast<int>(flag), text);
            break;
        }
        case Artist: {
            painter.setPen(otherColor);
            painter.setFont(fontT9);
            auto str = itemMeta.singer.isEmpty() ?
                       PlayListView::tr("Unknown artist") :
                       itemMeta.singer;
            QFont font(fontT9);
            QFontMetrics fm(font);
            auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
            painter.drawText(rect, static_cast<int>(flag), text);
            break;
        }
        case Album: {
            painter.setPen(otherColor);
            painter.setFont(fontT9);
            auto str = itemMeta.album.isEmpty() ?
                       PlayListView::tr("Unknown album") :
                       itemMeta.album;
            QFont font(fontT9);
            QFontMetrics fm(font);
            auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
            painter.drawText(rect, static_cast<int>(flag), text);
            break;
        }
        case Length: {
            painter.setPen(otherColor);
            painter.setFont(fontT9);
            painter.drawText(rect, static_cast<int>(flag), DMusic::lengthString(itemMeta.length));
            break;
        }
        default:
            break;
        }
    }

    painter.restore();
}

//QWidget *PlayItemDelegate::createEditor(QWidget *parent,
//                                        const QStyleOptionViewItem &option,
//                                        const QModelIndex &index) const

//{
//    return QStyledItemDelegate::createEditor(parent, option, index);
//}

//void PlayItemDelegate::setEditorData(QWidget *editor,
//                                     const QModelIndex &index) const
//{
//    QStyledItemDelegate::setEditorData(editor, index);
//}

//void PlayItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
//                                    const QModelIndex &index) const
//{
//    QStyledItemDelegate::setModelData(editor, model, index);
//}

void PlayItemDelegate::drawIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const PlayListView *listview = qobject_cast<const PlayListView *>(option.widget);

    QFont fontT9 = DFontSizeManager::instance()->get(DFontSizeManager::T9);
    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);

    MediaMeta activeMeta = Player::getInstance()->getActiveMeta();
    MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

//    auto background = option.palette.background();
//    background.setColor(Qt::red);
//    painter.fillRect(option.rect, background);

    // 绘制阴影
//    QRect shadowRect(option.rect.x() - 10 + xoffset, option.rect.y() + yoffset, 158, 148);
//    QPainterPath roundRectShadowPath;
//    roundRectShadowPath.addRoundRect(shadowRect, 8, 8);
//    painter.save();
//    painter.setClipPath(roundRectShadowPath);
//    painter.drawPixmap(shadowRect, m_shadowImg);
//    painter.restore();

    // 绘制圆角框
//    QRect rect(option.rect.x() + xoffset, option.rect.y() + xoffset, 140, 190);
//    QPainterPath roundRectPath;
//    roundRectPath.addRoundRect(rect, 10, 10);
//    painter.setClipPath(roundRectPath);

    // 绘制专辑图片
    painter.save();
    QIcon icon;
    auto value = index.data(Qt::DecorationRole);
    if (value.type() == QVariant::Icon) {
        icon = qvariant_cast<QIcon>(value);
    }
    QRect pixmapRect(option.rect.x() + xoffset, option.rect.y() + yoffset, ImgWidthAndHeight, ImgWidthAndHeight);
    QPainterPath roundPixmapRectPath;
    roundPixmapRectPath.addRoundRect(pixmapRect, roundRadius, roundRadius);
    painter.setClipPath(roundPixmapRectPath);
    painter.drawPixmap(pixmapRect, icon.pixmap(ImgWidthAndHeight, ImgWidthAndHeight));
    painter.restore();
    // 绘制图片上添加描边
    painter.save();
    QColor borderPenColor("#000000");
    borderPenColor.setAlphaF(0.05);
    QPen borderPen(borderPenColor);
    // 按设计修改为1
    borderPen.setWidthF(1);
    painter.setPen(borderPen);
    painter.drawRoundRect(pixmapRect/*.adjusted(1, 1, -1, 1)*/, roundRadius, roundRadius);
    painter.restore();

//    int startHeight = option.rect.y() + 159;
//    int fillAllHeight = 34;

    // 设置信息字体大小
    painter.setFont(fontT6);
    QFontMetrics fm(fontT6);
    QColor nameColor = "#000000";
    if (listview->getThemeType() == 2) {
        nameColor = "#C0C6D4";
    }
    painter.setPen(nameColor);
    // 调整歌曲名位置
    QRect nameFillRect(option.rect.x() + xoffset, option.rect.y() + yoffset + ImgWidthAndHeight,
                       ImgWidthAndHeight, fm.height());
    auto nameText = fm.elidedText(meta.title, Qt::ElideRight, ImgWidthAndHeight);
    painter.drawText(nameFillRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

    QFontMetrics extraNameFm(fontT9);
    painter.setFont(fontT9);
    nameColor.setAlphaF(0.5);
    painter.setPen(nameColor);
    // 调整歌手位置
    QRect extraNameFillRect(option.rect.x() + xoffset, nameFillRect.bottom(),
                            ImgWidthAndHeight - 51, extraNameFm.height());
    auto extraNameText = extraNameFm.elidedText(meta.singer, Qt::ElideRight, ImgWidthAndHeight - 51);
    painter.drawText(extraNameFillRect, Qt::AlignLeft | Qt::AlignVCenter, extraNameText);

    // 画时间矩形
    QString timeText = DMusic::lengthString(meta.length);
    // 如果时间超过一小时，矩形绘制长一点
    QRect timeFillRect;
    if (timeText.size() > 5) {
        timeFillRect = QRect(option.rect.x() + xoffset * 2 + ImgWidthAndHeight - 58,
                             extraNameFillRect.top() + extraNameFillRect.height() / 2 - 8,
                             48, 16);
    } else {
        timeFillRect = QRect(option.rect.x() + xoffset * 2 + ImgWidthAndHeight - 48,
                             extraNameFillRect.top() + extraNameFillRect.height() / 2 - 8,
                             38, 16);
    }
    painter.save();
    QColor timeFillColor("#232323");
    timeFillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        timeFillColor = "#DCDCDC";
        timeFillColor.setAlphaF(0.3);
    }
    painter.setPen(Qt::NoPen);
    painter.setBrush(timeFillColor);
    painter.drawRoundedRect(timeFillRect, 8, 8);
    painter.restore();

    // 时间字体固定大小
    fontT9.setPixelSize(11);
    painter.setFont(fontT9);

    // 画时间
    QColor timedColor = Qt::white;
    if (listview->getThemeType() == 2) {
        timedColor = "#C0C6D4";
    }
    painter.setPen(timedColor);
    painter.drawText(timeFillRect, Qt::AlignCenter, timeText);

    // 绘制选中时的阴影
    QBrush fillBrush(QColor(128, 128, 128, 0));
    if (option.state & QStyle::State_Selected) {
        fillBrush = QBrush(QColor(128, 128, 128, 90));
    }
    painter.save();
    painter.setClipPath(roundPixmapRectPath);
    painter.fillRect(pixmapRect, fillBrush);
    painter.restore();
    // 绘制选中时右上角的选中图标
//    if (option.state & QStyle::State_Selected) {
//        QRect selectionRect(option.rect.x() +  option.rect.width() - 20, option.rect.y() + 2, 14, 14);
//        painter.drawPixmap(selectionRect, m_selectedPix);
//    }
}

void PlayItemDelegate::drawListMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const PlayListView *listview = qobject_cast<const PlayListView *>(option.widget);

    QFont fontT9 = DFontSizeManager::instance()->get(DFontSizeManager::T9);
    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);

    QColor nameColor("#090909");
    QColor otherColor("#000000");

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QColor baseColor("#FFFFFF");
    baseColor.setAlphaF(0.1);
    QColor alternateBaseColor("#000000");
    alternateBaseColor.setAlphaF(0.02);
    QColor selecteColor("#000000");
    selecteColor.setAlphaF(0.20);
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

    otherColor.setAlphaF(0.5);
    if (listview->getThemeType() == 2) {
        nameColor = QColor("#C0C6D4");
        otherColor = QColor("#C0C6D4");
        otherColor.setAlphaF(0.6);
    }

//    auto hash = index.data().toString();
//    auto meta = MediaLibrary::instance()->meta(hash);
//    if (meta.isNull()) {
//        QString msg = "can not find " + hash;
//        qWarning() << msg;
//        return;
//    }
    MediaMeta activeMeta = Player::getInstance()->getActiveMeta();
    MediaMeta itemMeta = index.data(Qt::UserRole).value<MediaMeta>();
    if (activeMeta.hash == itemMeta.hash) {
        nameColor = QColor(DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
        otherColor = QColor(DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
        fontT6.setFamily("SourceHanSansSC");
        fontT6.setWeight(QFont::Medium);
    }

    if (option.state & QStyle::State_Selected) {
        painter.save();
        painter.setPen(Qt::NoPen);
        QColor selectColor(option.palette.highlight().color());
        painter.setBrush(selectColor);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
        painter.drawRoundedRect(selecteColorRect, 8, 8);
        painter.restore();

        nameColor = option.palette.highlightedText().color();
        otherColor = option.palette.highlightedText().color();
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

    for (int col = 0; col < ColumnButt; ++col) {
        auto flag = alignmentFlag(col);
        auto rect = colRect(col, option);
        switch (col) {
        case Number: {
            painter.setPen(otherColor);
            PlayListView *playListView = qobject_cast<PlayListView *>(const_cast<QWidget *>(option.widget));
            // Fixme:
            QFileInfo info(itemMeta.localPath);
            if (!info.exists() && itemMeta.mmType != MIMETYPE_CDA) {
                auto sz = QSizeF(20, 20);
                auto icon = QIcon::fromTheme("icon_warning").pixmap(sz.toSize());
                auto centerF = QRectF(rect).center();
                auto iconRect = QRectF(centerF.x() - sz.width() / 2,
                                       centerF.y() - sz.height() / 2,
                                       sz.width(), sz.height());
                painter.drawPixmap(iconRect, icon, QRectF());
                break;
            }

            //绘制播放动态图
            if (activeMeta.hash == itemMeta.hash) {
                QPixmap icon = playListView->getPlayPixmap(option.state & QStyle::State_Selected);
                auto centerF = QRectF(rect).center();
                qreal t_ratio = icon.devicePixelRatioF();
                QRect t_ratioRect;
                t_ratioRect.setX(0);
                t_ratioRect.setY(0);
                t_ratioRect.setWidth(static_cast<int>(icon.width() / t_ratio));
                t_ratioRect.setHeight(static_cast<int>(icon.height() / t_ratio));
                auto iconRect = QRectF(centerF.x() - t_ratioRect.width() / 2,
                                       centerF.y() - t_ratioRect.height() / 2,
                                       t_ratioRect.width(), t_ratioRect.height());
                painter.drawPixmap(iconRect.toRect(), icon);
            } else {
                painter.setFont(fontT9);
                // 只显示行号，如总数100,原来显示001修改为显示1
                auto str = QString::number(index.row() + 1);
                QFont font(fontT9);
                QFontMetrics fm(font);
                auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
                painter.drawText(rect, static_cast<int>(flag), text);
            }
            break;
        }
        case Title: {
            painter.setPen(nameColor);
            painter.setFont(fontT6);
            QFont font(fontT6);
            QFontMetrics fm(font);
            auto text = fm.elidedText(itemMeta.title, Qt::ElideMiddle, rect.width());
            painter.drawText(rect, static_cast<int>(flag), text);
            break;
        }
        case Artist: {
            painter.setPen(otherColor);
            painter.setFont(fontT9);
            auto str = itemMeta.singer.isEmpty() ?
                       PlayListView::tr("Unknown artist") :
                       itemMeta.singer;
            QFont font(fontT9);
            QFontMetrics fm(font);
            auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
            painter.drawText(rect, static_cast<int>(flag), text);
            break;
        }
        case Album: {
            painter.setPen(otherColor);
            painter.setFont(fontT9);
            auto str = itemMeta.album.isEmpty() ?
                       PlayListView::tr("Unknown album") :
                       itemMeta.album;
            QFont font(fontT9);
            QFontMetrics fm(font);
            auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
            painter.drawText(rect, static_cast<int>(flag), text);
            break;
        }
        case Length: {
            painter.setPen(otherColor);
            painter.setFont(fontT9);
            painter.drawText(rect, static_cast<int>(flag), DMusic::lengthString(itemMeta.length));
            break;
        }
        default:
            break;
        }
    }

    painter.restore();
}

PlayItemDelegate::PlayItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{
    setObjectName("PlayItemStyleProxy");
    m_shadowImg = DHiDPIHelper::loadNxPixmap(":/mpimage/light/shadow.svg");
    m_shadowImg = m_shadowImg.copy(5, 5, m_shadowImg.width() - 10, m_shadowImg.height() - 10);
    DStyle d;
    m_selectedPix = d.standardIcon(DStyle::SP_IndicatorChecked).pixmap(QSize(14, 14));
    m_unselectedPix = d.standardIcon(DStyle::SP_IndicatorUnchecked).pixmap(QSize(14, 14));
}

PlayItemDelegate::~PlayItemDelegate()
{
}

//QColor PlayItemDelegate::foreground(int col, const QStyleOptionViewItem &option) const
//{
//    if (option.state & QStyle::State_Selected) {
//        return m_highlightText;
//    }

//    auto emCol = static_cast<PlayItemDelegate::MusicColumn>(col);
//    switch (emCol) {
//    case PlayItemDelegate::Number:
//    case PlayItemDelegate::Artist:
//    case PlayItemDelegate::Album:
//    case PlayItemDelegate::Length:
//        return m_textColor;
//    case PlayItemDelegate::Title:
//        return m_numberColor;
//    case PlayItemDelegate::ColumnButt:
//        break;
//    }
//    return m_textColor;
//}

void PlayItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
//    Q_D(const PlayItemDelegate);
    QStyledItemDelegate::initStyleOption(option, index);
    option->state = option->state & ~QStyle::State_HasFocus;
}
