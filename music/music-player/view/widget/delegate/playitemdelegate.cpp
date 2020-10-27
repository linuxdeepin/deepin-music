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

#include "playitemdelegate.h"
#include "playitemdelegate_p.h"

#include <QDebug>
#include <QFont>
#include <DGuiApplicationHelper>
#include <QPainter>
#include <QStandardItemModel>
#include <QFileInfo>

#include <musicmeta.h>

#include <DHiDPIHelper>

#include "../playlistview.h"
#include "core/medialibrary.h"
#include "databaseservice.h"

DWIDGET_USE_NAMESPACE

const int PlayItemLeftMargin = 15;
const int PlayItemRightMargin = 20;
const int PlayItemNumberMargin = 10;

//static inline QString numberString(int row, const QStyleOptionViewItem &option)
//{
//    auto listview = qobject_cast<const MusicListView *>(option.widget);
//    auto itemCount = listview->model()->rowCount();
//    auto itemCountString = QString("%1").arg(itemCount);
//    return QString("%1").arg(int(row), itemCountString.length(), 10, QChar('0'));
//}

static inline int pixel2point(int pixel)
{
    return pixel * 96 / 72;
}

inline int headerPointWidth(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QFont measuringFont(option.font);
    QFontMetrics fm(measuringFont);
    auto headerWith = fm.width(QString("%1").arg(index.row()));
    return pixel2point(headerWith) + PlayItemLeftMargin + PlayItemNumberMargin;
}

inline int tailPointWidth(const QStyleOptionViewItem &option)
{
    QFont measuringFont(option.font);
    QFontMetrics fm(measuringFont);
    return pixel2point(fm.width("00:00")) + PlayItemRightMargin;
}

PlayItemDelegatePrivate::PlayItemDelegatePrivate(PlayItemDelegate *parent):
    QWidget(nullptr), q_ptr(parent)
{
    setObjectName("PlayItemStyleProxy");
    shadowImg = DHiDPIHelper::loadNxPixmap(":/mpimage/light/shadow.svg");
    shadowImg = shadowImg.copy(5, 5, shadowImg.width() - 10, shadowImg.height() - 10);
}

QColor PlayItemDelegatePrivate::textColor() const
{
    return m_textColor;
}
QColor PlayItemDelegatePrivate::titleColor() const
{
    return m_numberColor;
}
QColor PlayItemDelegatePrivate::highlightText() const
{
    return m_highlightText;
}
QColor PlayItemDelegatePrivate::background() const
{
    return m_background;
}
QColor PlayItemDelegatePrivate::alternateBackground() const
{
    return m_alternateBackground;
}
QColor PlayItemDelegatePrivate::highlightedBackground() const
{
    return m_highlightedBackground;
}

QString PlayItemDelegatePrivate::playingIcon() const
{
    return m_aimationPrefix;
}

QString PlayItemDelegatePrivate::highlightPlayingIcon() const
{
    return  m_highlightPlayingIcon;
}
void PlayItemDelegatePrivate::setTextColor(QColor textColor)
{
    m_textColor = textColor;
}
void PlayItemDelegatePrivate::setTitleColor(QColor numberColor)
{
    m_numberColor = numberColor;
}
void PlayItemDelegatePrivate::setHighlightText(QColor highlightText)
{
    m_highlightText = highlightText;
}
void PlayItemDelegatePrivate::setBackground(QColor background)
{
    m_background = background;
}
void PlayItemDelegatePrivate::setAlternateBackground(QColor alternateBackground)
{
    m_alternateBackground = alternateBackground;
}
void PlayItemDelegatePrivate::setHighlightedBackground(QColor highlightedBackground)
{
    m_highlightedBackground = highlightedBackground;
}

void PlayItemDelegatePrivate::setPlayingIcon(QString playingIcon)
{
    m_aimationPrefix = playingIcon;
}

void PlayItemDelegatePrivate::setHighlightPlayingIcon(QString highlightPlayingIcon)
{
    m_highlightPlayingIcon = highlightPlayingIcon;
}

QColor PlayItemDelegatePrivate::foreground(int col, const QStyleOptionViewItem &option) const
{
    if (option.state & QStyle::State_Selected) {
        return highlightText();
    }

    auto emCol = static_cast<PlayItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case PlayItemDelegate::Number:
    case PlayItemDelegate::Artist:
    case PlayItemDelegate::Album:
    case PlayItemDelegate::Length:
        return textColor();
    case PlayItemDelegate::Title:
        return titleColor();
    case PlayItemDelegate::ColumnButt:
        break;
    }
    return textColor();
}

//inline int PlayItemDelegatePrivate::timePropertyWidth(const QStyleOptionViewItem &option) const
//{
//    static auto width  = tailPointWidth(option);
//    return width;
//}

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
    if (DataBaseService::getInstance()->m_isPerformance) {
        performancePaint(painter, option, index);
        return;
    }
    auto listview = qobject_cast<const PlayListView *>(option.widget);
    if (listview->viewMode() == QListView::IconMode) {
        //QStyledItemDelegate::paint(painter, option, index);

        auto hash = index.data().toString();
        auto meta = MediaLibrary::instance()->meta(hash);
        if (meta.isNull())
            return;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::HighQualityAntialiasing);
        painter->setRenderHint(QPainter::SmoothPixmapTransform);

        auto background = option.palette.background();

        if (option.state & QStyle::State_Selected) {
//            background = option.palette.highlight();
        }

        painter->fillRect(option.rect, background);

        //绘制阴影
        QRect shadowRect(option.rect.x() - 10, option.rect.y(), 168, 158);
        QPainterPath roundRectShadowPath;
        roundRectShadowPath.addRoundRect(shadowRect, 8, 8);
        painter->save();
        painter->setClipPath(roundRectShadowPath);
        painter->drawPixmap(shadowRect, d_ptr->shadowImg);
        painter->restore();

        //绘制圆角框
        QRect rect(option.rect.x(), option.rect.y(), 150, 200);
        QPainterPath roundRectPath;
        roundRectPath.addRoundRect(rect, 10, 10);
        painter->setClipPath(roundRectPath);

        //绘制专辑图片
        auto icon = option.icon;
        auto value = index.data(Qt::DecorationRole);
        if (value.type() == QVariant::Icon) {
            icon = qvariant_cast<QIcon>(value);
        }
        QRect pixmapRect(option.rect.x(), option.rect.y(), 150, 150);
        painter->save();
        QPainterPath roundPixmapRectPath;
        roundPixmapRectPath.addRoundRect(pixmapRect, 10, 10);
        painter->setClipPath(roundPixmapRectPath);
        painter->drawPixmap(pixmapRect, icon.pixmap(option.rect.width(), option.rect.width()));

        //绘制图片上添加描边
        painter->save();
        QColor borderPenColor("#000000");
        borderPenColor.setAlphaF(0.05);
        QPen borderPen(borderPenColor);
        borderPen.setWidthF(2);
        painter->setPen(borderPen);
        painter->drawRoundRect(pixmapRect/*.adjusted(1, 1, -1, 1)*/, 10, 10);
        painter->restore();

        painter->restore();

        int startHeight = option.rect.y() + 159;
        int fillAllHeight = 34;

        //设置信息字体大小
        QFont font = option.font;
        font.setFamily("SourceHanSansSC");
        font.setWeight(QFont::Normal);
        font.setPixelSize(13);
        painter->setFont(font);
        QFontMetrics fm(font);
        QColor nameColor = "#000000";
        if (listview->getThemeType() == 2) {
            nameColor = "#C0C6D4";
        }
        painter->setPen(nameColor);
        QRect nameFillRect(option.rect.x(), startHeight - 5, option.rect.width(), fm.height());
        nameFillRect.adjust(8, 0, -7, 0);
        auto nameText = fm.elidedText(meta->title, Qt::ElideRight, 125);
        painter->drawText(nameFillRect, Qt::AlignLeft | Qt::AlignTop, nameText);

        font.setPixelSize(11);
        QFontMetrics extraNameFm(font);
        painter->setFont(font);
        nameColor.setAlphaF(1.0);
        painter->setPen(nameColor);
        QRect extraNameFillRect(option.rect.x(), startHeight + fillAllHeight / 2, 99, fm.height());
        extraNameFillRect.adjust(8, 0, -7, 0);
        auto extraNameText = extraNameFm.elidedText(meta->artist, Qt::ElideRight, 84);
        painter->drawText(extraNameFillRect, Qt::AlignLeft | Qt::AlignTop, extraNameText);

        //draw time
        QRect timeFillRect(option.rect.x() + 102, startHeight + 17, 38, 16);
        painter->save();
        QColor timeFillColor("#232323");
        timeFillColor.setAlphaF(0.3);
        if (listview->getThemeType() == 2) {
            timeFillColor = "#DCDCDC";
            timeFillColor.setAlphaF(0.3);
        }
        painter->setPen(Qt::NoPen);
        painter->setBrush(timeFillColor);
        painter->drawRoundedRect(timeFillRect, 8, 8);
        painter->restore();

        font.setPixelSize(10);
        painter->setFont(font);
        QColor timedColor = Qt::white;
        if (listview->getThemeType() == 2) {
            timedColor = "#C0C6D4";
        }
        painter->setPen(timedColor);
        auto timeText = fm.elidedText(DMusic::lengthString(meta->length), Qt::ElideRight, 38);
        painter->drawText(timeFillRect, Qt::AlignHCenter | Qt::AlignTop, timeText);

        QBrush t_fillBrush(QColor(128, 128, 128, 0));
        if (option.state & QStyle::State_Selected) {
            t_fillBrush = QBrush(QColor(128, 128, 128, 90));
        }
        painter->save();
        painter->setClipPath(roundPixmapRectPath);
        painter->fillRect(pixmapRect, t_fillBrush);
        painter->restore();

        painter->restore();

        return;
    }

    //Q_D(const PlayItemDelegate);

    painter->save();

    QFont font11 = option.font;
    font11.setFamily("SourceHanSansSC");
    font11.setWeight(QFont::Normal);
    font11.setPixelSize(11);
    QFont font14 = option.font;
    font14.setFamily("SourceHanSansSC");
    font14.setWeight(QFont::Normal);
    font14.setPixelSize(14);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

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
    //auto background = baseColor;

    int lrWidth = 10;
    if (!(option.state & QStyle::State_Selected) && !(option.state & QStyle::State_MouseOver)) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(background);
        //painter->drawRect(option.rect);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
        painter->drawRoundedRect(selecteColorRect, 8, 8);
        painter->restore();
    }

    QColor nameColor("#090909"), otherColor("#000000");
    otherColor.setAlphaF(0.5);
    if (listview->getThemeType() == 2) {
        nameColor = QColor("#C0C6D4");
        otherColor = QColor("#C0C6D4");
        otherColor.setAlphaF(0.6);
    }

    auto hash = index.data().toString();
    auto meta = MediaLibrary::instance()->meta(hash);
    if (meta.isNull()) {
        QString msg = "can not find " + hash;
        qWarning() << msg;
        return;
//        qFatal(msg.toStdString().c_str());
    }

    auto activeMeta = listview->activingMeta();
    if (!activeMeta.isNull() && activeMeta->hash == meta->hash) {
		 nameColor = QColor(DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
        otherColor = QColor("#2CA7F8");
        font14.setFamily("SourceHanSansSC");
        font14.setWeight(QFont::Medium);
    }

    if (option.state & QStyle::State_Selected) {
        painter->save();
        painter->setPen(Qt::NoPen);
//        QColor selectColor("#000000");
//        if (listview->getThemeType() == 2) {
//            selectColor = QColor("#FFFFFF");
//        }
//        selectColor.setAlphaF(0.2);
        QColor selectColor(option.palette.highlight().color());
        painter->setBrush(selectColor);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
        painter->drawRoundedRect(selecteColorRect, 8, 8);
        painter->restore();

        nameColor = option.palette.highlightedText().color();
        otherColor = option.palette.highlightedText().color();
    }/* else if ((index.row() % 2) == 0) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(alternateBaseColor);
        QRect selecteColorRect = option.rect.adjusted(5, 0, -5, 0);
        painter->drawRoundedRect(selecteColorRect, 8, 8);
        painter->restore();
    }*/

    if (option.state & QStyle::State_MouseOver) {
        painter->save();
        painter->setPen(Qt::NoPen);
        QColor hovertColor(option.palette.shadow().color());
        if (option.state & QStyle::State_Selected)
            hovertColor.setAlphaF(0.2);
        painter->setBrush(hovertColor);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
        painter->drawRoundedRect(selecteColorRect, 8, 8);
        painter->restore();
    }

    int rowCount = listview->model()->rowCount();
    auto rowCountSize = QString::number(rowCount).size();
    rowCountSize = qMax(rowCountSize, 3);

    for (int col = 0; col < ColumnButt; ++col) {
        auto flag = alignmentFlag(col);
        auto rect = colRect(col, option);
        switch (col) {
        case Number: {
            painter->setPen(otherColor);
            auto *listview = qobject_cast<PlayListView *>(const_cast<QWidget *>(option.widget));
            // Fixme:
            if (!meta.isNull() && meta->invalid) {
                auto sz = QSizeF(20, 20);
                auto icon = QIcon(":/mpimage/light/warning.svg").pixmap(sz.toSize());
                auto centerF = QRectF(rect).center();
                auto iconRect = QRectF(centerF.x() - sz.width() / 2,
                                       centerF.y() - sz.height() / 2,
                                       sz.width(), sz.height());
                painter->drawPixmap(iconRect, icon, QRectF());
                break;
            }

            if (!activeMeta.isNull() && activeMeta->hash == meta->hash) {
                auto icon = listview->getPlayPixmap();
                if (option.state & QStyle::State_Selected) {
                    icon = listview->getAlbumPixmap();
                }
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
                painter->drawPixmap(iconRect.toRect(), icon);

            } else {
                painter->setFont(font11);
                auto str = QString("%1").arg(index.row() + 1, rowCountSize, 10, QLatin1Char('0'));
                QFont font(font11);
                QFontMetrics fm(font);
                auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
                painter->drawText(rect, static_cast<int>(flag), text);
            }
            break;
        }
        case Title: {
            painter->setPen(nameColor);
            painter->setFont(font14);
            QFont font(font14);
            QFontMetrics fm(font);
            auto text = fm.elidedText(meta->title, Qt::ElideMiddle, rect.width());
            painter->drawText(rect, static_cast<int>(flag), text);
            break;
        }
        case Artist: {
            painter->setPen(nameColor);
            painter->setFont(font11);
            auto str = meta->artist.isEmpty() ?
                       PlayListView::tr("Unknown artist") :
                       meta->artist;
            QFont font(font11);
            QFontMetrics fm(font);
            auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
            painter->drawText(rect, static_cast<int>(flag), text);
            break;
        }
        case Album: {
            painter->setPen(nameColor);
            painter->setFont(font11);
            auto str = meta->album.isEmpty() ?
                       PlayListView::tr("Unknown album") :
                       meta->album;
            QFont font(font11);
            QFontMetrics fm(font);
            auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
            painter->drawText(rect, static_cast<int>(flag), text);
            break;
        }
        case Length:
            painter->setPen(otherColor);
            painter->setFont(font11);
            painter->drawText(rect, static_cast<int>(flag), DMusic::lengthString(meta->length));
            break;
        default:
            break;
        }
    }
    painter->restore();
}

void PlayItemDelegate::performancePaint(QPainter *painter,
                                        const QStyleOptionViewItem &option, const QModelIndex &index)const
{
    painter->save();

    MediaMeta meta = DataBaseService::getInstance()->m_MediaMetaMap[index.data().toString()];

    auto listview = qobject_cast<const PlayListView *>(option.widget);
    QFont font11 = option.font;
    font11.setFamily("SourceHanSansSC");
    font11.setWeight(QFont::Normal);
    font11.setPixelSize(11);
    QFont font14 = option.font;
    font14.setFamily("SourceHanSansSC");
    font14.setWeight(QFont::Normal);
    font14.setPixelSize(14);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

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
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(background);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
        painter->drawRoundedRect(selecteColorRect, 8, 8);
        painter->restore();
    }

    QColor nameColor("#090909"), otherColor("#000000");
    otherColor.setAlphaF(0.5);
    if (listview->getThemeType() == 2) {
        nameColor = QColor("#C0C6D4");
        otherColor = QColor("#C0C6D4");
        otherColor.setAlphaF(0.6);
    }

    auto hash = meta.hash;
//    auto meta = MediaLibrary::instance()->meta(hash);

    auto activeMeta = listview->activingMeta();
    if (!activeMeta.isNull() && activeMeta->hash == meta.hash) {
        nameColor = QColor("#2CA7F8");
        otherColor = QColor("#2CA7F8");
        font14.setFamily("SourceHanSansSC");
        font14.setWeight(QFont::Medium);
    }

    if (option.state & QStyle::State_Selected) {
        painter->save();
        painter->setPen(Qt::NoPen);

        QColor selectColor(option.palette.highlight().color());
        painter->setBrush(selectColor);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
        painter->drawRoundedRect(selecteColorRect, 8, 8);
        painter->restore();

        nameColor = option.palette.highlightedText().color();
        otherColor = option.palette.highlightedText().color();
    }

    if (option.state & QStyle::State_MouseOver) {
        painter->save();
        painter->setPen(Qt::NoPen);
        QColor hovertColor(option.palette.shadow().color());
        if (option.state & QStyle::State_Selected)
            hovertColor.setAlphaF(0.2);
        painter->setBrush(hovertColor);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
        painter->drawRoundedRect(selecteColorRect, 8, 8);
        painter->restore();
    }

    int rowCount = listview->model()->rowCount();
    auto rowCountSize = QString::number(rowCount).size();
    rowCountSize = qMax(rowCountSize, 3);

    for (int col = 0; col < ColumnButt; ++col) {
        auto flag = alignmentFlag(col);
        auto rect = colRect(col, option);
        switch (col) {
        case Number: {
            painter->setPen(otherColor);
            auto *listview = qobject_cast<PlayListView *>(const_cast<QWidget *>(option.widget));
            // Fixme:
            QFileInfo info(meta.localPath);
            if (!info.exists()) {
                auto sz = QSizeF(15, 15);
                auto icon = QIcon(":/mpimage/light/warning.svg").pixmap(sz.toSize());
                auto centerF = QRectF(rect).center();
                auto iconRect = QRectF(centerF.x() - sz.width() / 2,
                                       centerF.y() - sz.height() / 2,
                                       sz.width(), sz.height());
                painter->drawPixmap(iconRect, icon, QRectF());
            }

            if (!activeMeta.isNull() && activeMeta->hash == meta.hash) {
                auto icon = listview->getPlayPixmap();
                if (option.state & QStyle::State_Selected) {
                    icon = listview->getAlbumPixmap();
                }
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
                painter->drawPixmap(iconRect.toRect(), icon);

            } else {
                painter->setFont(font11);
                auto str = QString("%1").arg(index.row() + 1, rowCountSize, 10, QLatin1Char('0'));
                QFont font(font11);
                QFontMetrics fm(font);
                auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
                painter->drawText(rect, static_cast<int>(flag), text);
            }
            break;
        }
        case Title: {
            painter->setPen(nameColor);
            painter->setFont(font14);
            QFont font(font14);
            QFontMetrics fm(font);
            auto text = fm.elidedText(meta.title, Qt::ElideMiddle, rect.width());
            painter->drawText(rect, static_cast<int>(flag), text);
            break;
        }
        case Artist: {
            painter->setPen(otherColor);
            painter->setFont(font11);
            auto str = meta.artist.isEmpty() ?
                       PlayListView::tr("Unknown artist") :
                       meta.artist;
            QFont font(font11);
            QFontMetrics fm(font);
            auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
            painter->drawText(rect, static_cast<int>(flag), text);
            break;
        }
        case Album: {
            painter->setPen(otherColor);
            painter->setFont(font11);
            auto str = meta.album.isEmpty() ?
                       PlayListView::tr("Unknown album") :
                       meta.album;
            QFont font(font11);
            QFontMetrics fm(font);
            auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
            painter->drawText(rect, static_cast<int>(flag), text);
            break;
        }
        case Length:
            painter->setPen(otherColor);
            painter->setFont(font11);
            painter->drawText(rect, static_cast<int>(flag), DMusic::lengthString(meta.length));
            break;
        default:
            break;
        }
    }
    painter->restore();
}

QSize PlayItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    //Q_D(const PlayItemDelegate);
    auto listview = qobject_cast<const PlayListView *>(option.widget);
    if (listview->viewMode() == QListView::IconMode) {
        return QSize(150, 200);
    } else {
        auto baseSize = QStyledItemDelegate::sizeHint(option, index);
        return QSize(baseSize.width(), 38);
    }
//        return QStyledItemDelegate::sizeHint(option, index);

//    auto listview = qobject_cast<const PlayListView *>(option.widget);
//    if (listview->viewMode() == QListView::IconMode) {
//        return QStyledItemDelegate::sizeHint(option, index);
//    }

//    auto baseSize = QStyledItemDelegate::sizeHint(option, index);
//    return  QSize(baseSize.width() / 5, baseSize.height());
////    auto headerWidth = headerPointWidth(option, index);
//    auto headerWidth = 17 + 10 + 10 + 4;
//    auto tialWidth = d->timePropertyWidth(option);
//    auto w = option.widget->width() - headerWidth - tialWidth;
//    Q_ASSERT(w > 0);
//    switch (index.column()) {
//    case 0:
//        return  QSize(headerWidth, baseSize.height());
//    case 1:
//        return  QSize(w / 2, baseSize.height());
//    case 2:
//    case 3:
//        return  QSize(w / 4, baseSize.height());
//    case 4:
//        return  QSize(tialWidth, baseSize.height());
//    }

//    return baseSize;
}

QWidget *PlayItemDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const

{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void PlayItemDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}

void PlayItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

PlayItemDelegate::PlayItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent), d_ptr(new PlayItemDelegatePrivate(this))
{

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
