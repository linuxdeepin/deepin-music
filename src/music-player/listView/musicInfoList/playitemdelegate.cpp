// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include "playlistmodel.h"

DWIDGET_USE_NAMESPACE

#ifdef DTKWIDGET_CLASS_DSizeMode
const int PlayItemCompactRightMargin = 15;
const int ImgCompactWidthAndHeight = 126;
#endif
const int PlayItemRightMargin = 20;
const int ImgWidthAndHeight = 150;
const int xoffset = 5;
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

static int imgWidthAndHeight() {
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
        return ImgCompactWidthAndHeight;
    } else {
        return ImgWidthAndHeight;
    }
#else
    return ImgWidthAndHeight;
#endif
}
void PlayItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    auto listview = qobject_cast<const PlayListView *>(option.widget);
    if (listview == nullptr) return;
    QColor color = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
    QPen pen(color, 1);
    painter->save();
    painter->setPen(pen);
    int curRowCount = listview->m_model->rowCount();
    if (listview->viewMode() == QListView::IconMode) {
        if (CommonService::getInstance()->isTabletEnvironment()) {
            drawTabletIconMode(*painter, option, index);
        } else {
            drawIconMode(*painter, option, index);
            // 绘制拖拽分割线
            if (listview->m_dragFlag && listview->m_isDraging) {
                if (listview->highlightedRow() == index.row()) {
                    painter->drawLine(QLine(QPoint(option.rect.x() - 3, option.rect.top() + yoffset),
                                            QPoint(option.rect.x() - 3, option.rect.y() + yoffset + imgWidthAndHeight())));
                } else if ((index.row() == (curRowCount - 1)) && (listview->highlightedRow() == curRowCount || listview->highlightedRow() == -1)) {
                    painter->drawLine(QLine(QPoint(option.rect.right() + 1, option.rect.top() + yoffset),
                                            QPoint(option.rect.right() + 1, option.rect.y() + yoffset + imgWidthAndHeight())));
                }
            }
        }
    } else {
        if (CommonService::getInstance()->isTabletEnvironment()) {
            drawTabletListMode(*painter, option, index);
        } else {
            drawListMode(*painter, option, index);
            // 绘制拖拽分割线
            if (!listview->allSelectedIndexes().contains(index) && listview->m_dragFlag && listview->m_isDraging) {
                int lrWidth = 10;
                if (listview->highlightedRow() == index.row()) {
                    painter->drawLine(QLine(QPoint(option.rect.x() + lrWidth, option.rect.top() + 1), QPoint(option.rect.width() - lrWidth, option.rect.top() + 1)));
                } else if ((index.row() == (curRowCount - 1)) && (listview->highlightedRow() == curRowCount || listview->highlightedRow() == -1)) {
                    painter->drawLine(QLine(QPoint(option.rect.x() + lrWidth, option.rect.bottom() - 1), QPoint(option.rect.width() - lrWidth, option.rect.bottom() - 1)));
                }
            }
        }
    }
    painter->restore();
}

QSize PlayItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    auto *listview = qobject_cast<const PlayListView *>(option.widget);
    if (listview && listview->viewMode() == QListView::IconMode) {
        if (CommonService::getInstance()->isTabletEnvironment()) {
            return QSize(200, 243);
        } else {
#ifdef DTKWIDGET_CLASS_DSizeMode
            if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
                return QSize(ImgCompactWidthAndHeight + xoffset * 2, 190);
            } else
#endif
            {
                return QSize(ImgWidthAndHeight + xoffset * 2, 210);
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

bool PlayItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    const PlayListView *listview = qobject_cast<const PlayListView *>(option.widget);

    if (index.isValid() && listview->viewMode() == QListView::IconMode) {
        const QMouseEvent *pressEvent = static_cast<QMouseEvent *>(event);
        const QPointF pressPos = pressEvent->pos();
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
            if ((!info.exists() || !Player::getInstance()->supportedSuffixStrList().contains(info.suffix().toLower())) && itemMeta.mmType != MIMETYPE_CDA) {
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

void PlayItemDelegate::drawIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const PlayListView *listview = qobject_cast<const PlayListView *>(option.widget);
    MediaMeta activeMeta = Player::getInstance()->getActiveMeta();
    MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 绘制专辑图片
    painter.save();
    QIcon icon;
    QVariant value = index.data(Qt::DecorationRole);
    if (value.type() == QVariant::Icon) {
        icon = qvariant_cast<QIcon>(value);
    }
    QRect pixmapRect(option.rect.x() + xoffset, option.rect.y() + yoffset, imgWidthAndHeight(), imgWidthAndHeight());
    // 绘制选中时效果
    QBrush fillBrush(QColor(128, 128, 128, 0));
    if (option.state & QStyle::State_Selected) {
        fillBrush = QBrush(option.palette.highlight().color());
    }
    int borderWidth = 4;
    painter.save();
    QPainterPath borderPixmapRectPath;
    borderPixmapRectPath.addRoundRect(pixmapRect.adjusted(-borderWidth, -borderWidth, borderWidth, borderWidth), roundRadius, roundRadius);
    painter.setClipPath(borderPixmapRectPath);
    painter.fillRect(pixmapRect.adjusted(-borderWidth, -borderWidth, borderWidth, borderWidth), fillBrush);
    painter.restore();
    // 绘制图片
    QPainterPath roundPixmapRectPath;
    roundPixmapRectPath.addRoundRect(pixmapRect, roundRadius, roundRadius);
    painter.setClipPath(roundPixmapRectPath);
    painter.drawPixmap(pixmapRect, icon.pixmap(imgWidthAndHeight(), imgWidthAndHeight()));
    painter.restore();
    // 绘制边框
    painter.save();
    QColor borderPenColor("#000000");
    borderPenColor.setAlphaF(0.05);
    QPen borderPen(borderPenColor);
    // 按设计修改为1
    borderPen.setWidthF(1);
    painter.setPen(borderPen);
    painter.drawRoundRect(pixmapRect, roundRadius, roundRadius);
    painter.restore();

    // 绘制歌曲信息
    painter.save();
    QRect infoRect(option.rect.x() + xoffset, option.rect.y() + yoffset + imgWidthAndHeight(),
                   imgWidthAndHeight(), option.rect.height() - imgWidthAndHeight() - yoffset * 2);
    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    QFontMetrics fm(fontT6);
    QColor nameColor = "#000000";
    QRect nameFillRect(infoRect.x(), infoRect.y(), imgWidthAndHeight(), fm.height());
    QString nameText = fm.elidedText(meta.title, Qt::ElideRight, imgWidthAndHeight());
    if (listview->getThemeType() == 2) {
        nameColor = "#C0C6D4";
    }
    painter.setFont(fontT6);
    painter.setPen(nameColor);
    painter.drawText(nameFillRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

    QFont fontT9 = DFontSizeManager::instance()->get(DFontSizeManager::T9);
    QFontMetrics extraNameFm(fontT9);
    QRect extraNameFillRect(infoRect.x(), nameFillRect.bottom(),
                            imgWidthAndHeight() - 50, extraNameFm.height());
    QString extraNameText = extraNameFm.elidedText(meta.singer, Qt::ElideRight, imgWidthAndHeight() - 51);
    painter.setFont(fontT9);
    nameColor.setAlphaF(0.5);
    painter.setPen(nameColor);
    painter.drawText(extraNameFillRect, Qt::AlignLeft | Qt::AlignVCenter, extraNameText);

    // 绘制时间
    QString timeText = DMusic::lengthString(meta.length);
    QRect timeFillRect;
    int timeRectHeight = 16;
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
        timeRectHeight = 13;
    }
#endif
    if (timeText.size() > 5) {
        // 如果时间超过一小时，矩形绘制长一点
        timeFillRect = QRect(infoRect.x() + imgWidthAndHeight() - 48, infoRect.y() + nameFillRect.height(),
                             48, timeRectHeight);
    } else {
        timeFillRect = QRect(infoRect.x() + imgWidthAndHeight() - 38, infoRect.y() + nameFillRect.height(),
                             38, timeRectHeight);
    }
    painter.restore();

    painter.save();
    QColor timeFillColor("#232323");
    timeFillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        timeFillColor = "#DCDCDC";
        timeFillColor.setAlphaF(0.3);
    }
    painter.setPen(Qt::NoPen);
    painter.setBrush(timeFillColor);
    painter.drawRoundedRect(timeFillRect, timeRectHeight / 2, timeRectHeight / 2);
    painter.restore();

    painter.save();
    // 时间字体固定大小
    fontT9.setPixelSize(11);
    painter.setFont(fontT9);
    QColor timedColor = Qt::white;
    if (listview->getThemeType() == 2) {
        timedColor = "#C0C6D4";
    }
    painter.setPen(timedColor);
    painter.drawText(timeFillRect, Qt::AlignCenter, timeText);
    painter.restore();
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
    QColor background = (index.row() % 2) == 1 ? baseColor : alternateBaseColor;
    int lrWidth = 10;
    int tHeight = 0;
    if (!(option.state & QStyle::State_Selected) && !(option.state & QStyle::State_MouseOver)) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(background);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, tHeight, -lrWidth, 0);
        painter.drawRoundedRect(selecteColorRect, 8, 8);
        painter.restore();
    }

    otherColor.setAlphaF(0.5);
    if (listview->getThemeType() == 2) {
        nameColor = QColor("#C0C6D4");
        otherColor = QColor("#C0C6D4");
        otherColor.setAlphaF(0.6);
    }

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
        QRect selecteColorRect = option.rect.adjusted(lrWidth, tHeight, -lrWidth, 0);
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
        QRect selecteColorRect = option.rect.adjusted(lrWidth, tHeight, -lrWidth, 0);
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
            if ((!info.exists() || !Player::getInstance()->supportedSuffixStrList().contains(info.suffix().toLower())) && itemMeta.mmType != MIMETYPE_CDA) {
                QSizeF sz = QSizeF(20, 20);
#ifdef DTKWIDGET_CLASS_DSizeMode
                if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
                    // 播放图标缩小80%
                    sz.setWidth(16);
                    sz.setHeight(16);
                }
#endif
                QPixmap icon = QIcon::fromTheme("icon_warning").pixmap(sz.toSize());
                QPointF centerF = QRectF(rect).center();
                QRectF iconRect = QRectF(centerF.x() - sz.width() / 2,
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

void PlayItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
//    Q_D(const PlayItemDelegate);
    QStyledItemDelegate::initStyleOption(option, index);
    option->state = option->state & ~QStyle::State_HasFocus;
}
