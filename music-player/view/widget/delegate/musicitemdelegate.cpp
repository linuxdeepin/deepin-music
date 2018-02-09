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

#include "musicitemdelegate.h"
#include "musicitemdelegate_p.h"

#include <QDebug>
#include <QFont>
#include <QPainter>
#include <QStandardItemModel>

#include <DThemeManager>

#include <musicmeta.h>

#include "../musiclistview.h"
#include "core/medialibrary.h"

DWIDGET_USE_NAMESPACE

const int MusicItemLeftMargin = 15;
const int MusicItemRightMargin = 20;
const int MusicItemNumberMargin = 10;

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
    return pixel2point(headerWith) + MusicItemLeftMargin + MusicItemNumberMargin;
}

inline int tailPointWidth(const QStyleOptionViewItem &option)
{
    QFont measuringFont(option.font);
    QFontMetrics fm(measuringFont);
    return pixel2point(fm.width("00:00")) + MusicItemRightMargin;
}

MusicItemDelegatePrivate::MusicItemDelegatePrivate(MusicItemDelegate *parent):
    QWidget(nullptr), q_ptr(parent)
{
    setObjectName("MusicItemStyleProxy");
    DThemeManager::instance()->registerWidget(this, "MusicItemStyleProxy");
}

QColor MusicItemDelegatePrivate::textColor() const
{
    return m_textColor;
}
QColor MusicItemDelegatePrivate::titleColor() const
{
    return m_numberColor;
}
QColor MusicItemDelegatePrivate::highlightText() const
{
    return m_highlightText;
}
QColor MusicItemDelegatePrivate::background() const
{
    return m_background;
}
QColor MusicItemDelegatePrivate::alternateBackground() const
{
    return m_alternateBackground;
}
QColor MusicItemDelegatePrivate::highlightedBackground() const
{
    return m_highlightedBackground;
}

QString MusicItemDelegatePrivate::playingIcon() const
{
    return m_aimationPrefix;
}

QString MusicItemDelegatePrivate::highlightPlayingIcon() const
{
    return  m_highlightPlayingIcon;
}
void MusicItemDelegatePrivate::setTextColor(QColor textColor)
{
    m_textColor = textColor;
}
void MusicItemDelegatePrivate::setTitleColor(QColor numberColor)
{
    m_numberColor = numberColor;
}
void MusicItemDelegatePrivate::setHighlightText(QColor highlightText)
{
    m_highlightText = highlightText;
}
void MusicItemDelegatePrivate::setBackground(QColor background)
{
    m_background = background;
}
void MusicItemDelegatePrivate::setAlternateBackground(QColor alternateBackground)
{
    m_alternateBackground = alternateBackground;
}
void MusicItemDelegatePrivate::setHighlightedBackground(QColor highlightedBackground)
{
    m_highlightedBackground = highlightedBackground;
}

void MusicItemDelegatePrivate::setPlayingIcon(QString playingIcon)
{
    m_aimationPrefix = playingIcon;
}

void MusicItemDelegatePrivate::setHighlightPlayingIcon(QString highlightPlayingIcon)
{
    m_highlightPlayingIcon = highlightPlayingIcon;
}

QColor MusicItemDelegatePrivate::foreground(int col, const QStyleOptionViewItem &option) const
{
    if (option.state & QStyle::State_Selected) {
        return highlightText();
    }

    auto emCol = static_cast<MusicItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicItemDelegate::Number:
    case MusicItemDelegate::Artist:
    case MusicItemDelegate::Album:
    case MusicItemDelegate::Length:
        return textColor();
    case MusicItemDelegate::Title:
        return titleColor();
    case MusicItemDelegate::ColumnButt:
        break;
    }
    return textColor();
}

inline int MusicItemDelegatePrivate::timePropertyWidth(const QStyleOptionViewItem &option) const
{
    static auto width  = tailPointWidth(option);
    return width;
}

static inline QFlags<Qt::AlignmentFlag> alignmentFlag(int col)
{
    auto emCol = static_cast<MusicItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicItemDelegate::Number:
    case MusicItemDelegate::Title:
    case MusicItemDelegate::Artist:
    case MusicItemDelegate::Album:
        return (Qt::AlignLeft | Qt::AlignVCenter);
    case MusicItemDelegate::Length:
        return (Qt::AlignRight | Qt::AlignVCenter);
    case MusicItemDelegate::ColumnButt:
        break;
    }
    return (Qt::AlignLeft | Qt::AlignVCenter);;
}

static inline QRect colRect(int col, const QStyleOptionViewItem &option)
{
    static auto tailwidth  = tailPointWidth(option) + 20;
    auto w = option.rect.width() - 0 - tailwidth;

    auto emCol = static_cast<MusicItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicItemDelegate::Number:
        return QRect(0, option.rect.y(), 40, option.rect.height());
    case MusicItemDelegate::Title:
        return QRect(40, option.rect.y(), w / 2 - 20, option.rect.height());
    case MusicItemDelegate::Artist:
        return QRect(40 + w / 2, option.rect.y(), w / 4 - 20, option.rect.height());
    case MusicItemDelegate::Album:
        return QRect(40 + w / 2 + w / 4, option.rect.y(), w / 4 - 20, option.rect.height());
    case MusicItemDelegate::Length:
        return QRect(w, option.rect.y(), tailwidth - 20, option.rect.height());
    case MusicItemDelegate::ColumnButt:
        break;
    }
    return option.rect.marginsRemoved(QMargins(0, 0, 0, 0));
}


void MusicItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_D(const MusicItemDelegate);

    painter->save();

    QFont font11 = option.font;
    font11.setPixelSize(11);
    QFont font12 = option.font;
    font12.setPixelSize(12);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);

    auto background = (index.row() % 2) == 0 ? d->background() : d->alternateBackground();

    if (option.state & QStyle::State_Selected) {
        background = d->highlightedBackground();
    }

    if (option.state & QStyle::State_HasFocus) {
//        background = Qt::red;
    }

    painter->fillRect(option.rect, background);
//    painter->setPen(Qt::red);
//    painter->drawRect(option.rect);

    auto hash = index.data().toString();
    auto meta = MediaLibrary::instance()->meta(hash);
    if (meta.isNull()) {
        QString msg = "can not find " + hash;
        qWarning() << msg;
        return;
//        qFatal(msg.toStdString().c_str());
    }

    for (int col = 0; col < ColumnButt; ++col) {
        auto textColor = d->foreground(col, option);
        auto flag = alignmentFlag(col);
        auto rect = colRect(col, option);
        painter->setPen(textColor);
        switch (col) {
        case Number: {
            auto *listview = qobject_cast<MusicListView *>(const_cast<QWidget *>(option.widget));
            // Fixme:
            auto activeMeta = listview->activingMeta();
            if (!meta.isNull() && meta->invalid) {
                auto sz = QSizeF(15, 15);
                auto icon = QIcon(":/common/image/warning.svg").pixmap(sz.toSize());
                auto centerF = QRectF(rect).center();
                auto iconRect = QRectF(centerF.x() - sz.width() / 2,
                                       centerF.y() - sz.height() / 2,
                                       sz.width(), sz.height());
                painter->drawPixmap(iconRect, icon, QRectF());
                break;
            }

            if (activeMeta == meta) {
                auto playingIcon = d->playingIcon();
                if (option.state & QStyle::State_Selected) {
                    playingIcon = d->highlightPlayingIcon();
                }
                auto icon = QPixmap(playingIcon);
                auto centerF = QRectF(rect).center();
                auto iconRect = QRectF(centerF.x() - icon.width() / 2,
                                       centerF.y() - icon.height() / 2,
                                       icon.width(), icon.height());
                painter->drawPixmap(iconRect.toRect(), icon);
            }
            break;
        }
        case Title: {
            painter->setFont(font12);
            QFont font(font12);
            QFontMetrics fm(font);
            auto text = fm.elidedText(meta->title, Qt::ElideMiddle, rect.width());
            painter->drawText(rect, flag, text);
            break;
        }
        case Artist: {
            painter->setFont(font11);
            auto str = meta->artist.isEmpty() ?
                       MusicListView::tr("Unknown artist") :
                       meta->artist;
            QFont font(font11);
            QFontMetrics fm(font);
            auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
            painter->drawText(rect, flag, text);
            break;
        }
        case Album: {
            painter->setFont(font11);
            auto str = meta->album.isEmpty() ?
                       MusicListView::tr("Unknown album") :
                       meta->album;
            QFont font(font11);
            QFontMetrics fm(font);
            auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
            painter->drawText(rect, flag, text);
            break;
        }
        case Length:
            painter->setFont(font11);
            painter->drawText(rect, flag, DMusic::lengthString(meta->length));
            break;
        default:
            break;
        }
    }
    painter->restore();
}

QSize MusicItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    Q_D(const MusicItemDelegate);

    auto baseSize = QStyledItemDelegate::sizeHint(option, index);
    return  QSize(baseSize.width() / 5, baseSize.height());
//    auto headerWidth = headerPointWidth(option, index);
    auto headerWidth = 17 + 10 + 10 + 4;
    auto tialWidth = d->timePropertyWidth(option);
    auto w = option.widget->width() - headerWidth - tialWidth;
    Q_ASSERT(w > 0);
    switch (index.column()) {
    case 0:
        return  QSize(headerWidth, baseSize.height());
    case 1:
        return  QSize(w / 2, baseSize.height());
    case 2:
    case 3:
        return  QSize(w / 4, baseSize.height());
    case 4:
        return  QSize(tialWidth, baseSize.height());
    }

    return baseSize;
}

QWidget *MusicItemDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const

{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void MusicItemDelegate::setEditorData(QWidget *editor,
                                      const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}

void MusicItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

MusicItemDelegate::MusicItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent), d_ptr(new MusicItemDelegatePrivate(this))
{

}

MusicItemDelegate::~MusicItemDelegate()
{
}

void MusicItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
//    Q_D(const MusicItemDelegate);
    QStyledItemDelegate::initStyleOption(option, index);
    option->state = option->state & ~QStyle::State_HasFocus;
}
