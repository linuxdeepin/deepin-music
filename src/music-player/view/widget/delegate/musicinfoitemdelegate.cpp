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

#include "musicinfoitemdelegate.h"
#include "musicinfoitemdelegate_p.h"

#include <QDebug>
#include <QFont>
#include <QPainter>
#include <QStandardItemModel>

#include <musicmeta.h>

#include "../musiclistinfoview.h"
#include "core/medialibrary.h"

DWIDGET_USE_NAMESPACE

const int PlayItemLeftMargin = 15;
const int PlayItemRightMargin = 20;
const int PlayItemNumberMargin = 10;

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

MusicInfoItemDelegatePrivate::MusicInfoItemDelegatePrivate(MusicInfoItemDelegate *parent):
    QWidget(nullptr), q_ptr(parent)
{
}

QColor MusicInfoItemDelegatePrivate::textColor() const
{
    return m_textColor;
}
QColor MusicInfoItemDelegatePrivate::titleColor() const
{
    return m_numberColor;
}
QColor MusicInfoItemDelegatePrivate::highlightText() const
{
    return m_highlightText;
}
QColor MusicInfoItemDelegatePrivate::background() const
{
    return m_background;
}
QColor MusicInfoItemDelegatePrivate::alternateBackground() const
{
    return m_alternateBackground;
}
QColor MusicInfoItemDelegatePrivate::highlightedBackground() const
{
    return m_highlightedBackground;
}

QString MusicInfoItemDelegatePrivate::playingIcon() const
{
    return m_aimationPrefix;
}

QString MusicInfoItemDelegatePrivate::highlightPlayingIcon() const
{
    return  m_highlightPlayingIcon;
}
void MusicInfoItemDelegatePrivate::setTextColor(QColor textColor)
{
    m_textColor = textColor;
}
void MusicInfoItemDelegatePrivate::setTitleColor(QColor numberColor)
{
    m_numberColor = numberColor;
}
void MusicInfoItemDelegatePrivate::setHighlightText(QColor highlightText)
{
    m_highlightText = highlightText;
}
void MusicInfoItemDelegatePrivate::setBackground(QColor background)
{
    m_background = background;
}
void MusicInfoItemDelegatePrivate::setAlternateBackground(QColor alternateBackground)
{
    m_alternateBackground = alternateBackground;
}
void MusicInfoItemDelegatePrivate::setHighlightedBackground(QColor highlightedBackground)
{
    m_highlightedBackground = highlightedBackground;
}

void MusicInfoItemDelegatePrivate::setPlayingIcon(QString playingIcon)
{
    m_aimationPrefix = playingIcon;
}

void MusicInfoItemDelegatePrivate::setHighlightPlayingIcon(QString highlightPlayingIcon)
{
    m_highlightPlayingIcon = highlightPlayingIcon;
}

QColor MusicInfoItemDelegatePrivate::foreground(int col, const QStyleOptionViewItem &option) const
{
    if (option.state & QStyle::State_Selected) {
        return highlightText();
    }

    auto emCol = static_cast<MusicInfoItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicInfoItemDelegate::Number:
    case MusicInfoItemDelegate::Length:
        return textColor();
    case MusicInfoItemDelegate::Title:
        return titleColor();
    default:
        break;
    }
    return textColor();
}

inline int MusicInfoItemDelegatePrivate::timePropertyWidth(const QStyleOptionViewItem &option) const
{
    static auto width  = tailPointWidth(option);
    return width;
}

static inline QFlags<Qt::AlignmentFlag> alignmentFlag(int col)
{
    auto emCol = static_cast<MusicInfoItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicInfoItemDelegate::Number:
        return Qt::AlignCenter;
    case MusicInfoItemDelegate::Title:
        return (Qt::AlignLeft | Qt::AlignVCenter);
    case MusicInfoItemDelegate::Length:
        return (Qt::AlignRight | Qt::AlignVCenter);
    default:
        break;
    }
    return (Qt::AlignLeft | Qt::AlignVCenter);;
}

static inline QRect colRect(int col, const QStyleOptionViewItem &option)
{
    static auto tailwidth  = tailPointWidth(option) + 20;
    auto w = option.rect.width() - 0 - tailwidth;

    auto emCol = static_cast<MusicInfoItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicInfoItemDelegate::Number:
        return QRect(0, option.rect.y(), 40, option.rect.height());
    case MusicInfoItemDelegate::Title:
        return QRect(40, option.rect.y(), w / 2 - 20, option.rect.height());
    case MusicInfoItemDelegate::Length:
        return QRect(w, option.rect.y(), tailwidth - 20, option.rect.height());
    default:
        break;
    }
    return option.rect.marginsRemoved(QMargins(0, 0, 0, 0));
}


void MusicInfoItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    auto listview = qobject_cast<const MusicListInfoView *>(option.widget);

    Q_D(const MusicInfoItemDelegate);

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

    auto background = (index.row() % 2) == 0 ? baseColor : alternateBaseColor;

    if (option.state & QStyle::State_Selected) {
        background = selecteColor;
    }

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(background);
    painter->drawRect(option.rect);
    painter->restore();
    //painter->fillRect(option.rect, background);

    int rowCount = listview->model()->rowCount();
    auto rowCountSize = QString::number(rowCount).size();
    rowCountSize = qMax(rowCountSize, 2);

    auto hash = index.data().toString();
    auto meta = MediaLibrary::instance()->meta(hash);
    if (meta.isNull()) {
        QString msg = "can not find " + hash;
        qWarning() << msg;
        return;
    }

    QColor nameColor("#090909"), otherColor("#797979");
    if (listview->getThemeType() == 2) {
        nameColor = QColor("#C0C6D4");
        otherColor = QColor("#C0C6D4");
    }

    for (int col = 0; col < 3; ++col) {
        auto textColor = d->foreground(col, option);
        QColor brightTextColor(option.palette.highlight().color());
        auto flag = alignmentFlag(col);
        auto rect = colRect(col, option);
        auto activeMeta = listview->playing();
        if (activeMeta == meta) {
            nameColor = QColor("#2CA7F8");
            otherColor = QColor("#2CA7F8");
            font14.setFamily("SourceHanSansSC");
            font14.setWeight(QFont::Medium);
        }
        switch (col) {
        case Number: {
            // Fixme:
            painter->setPen(otherColor);
            if (!meta.isNull() && meta->invalid) {
                auto sz = QSizeF(15, 15);
                auto icon = QIcon(":/mpimage/light/warning.svg").pixmap(sz.toSize());
                auto centerF = QRectF(rect).center();
                auto iconRect = QRectF(centerF.x() - sz.width() / 2,
                                       centerF.y() - sz.height() / 2,
                                       sz.width(), sz.height());
                painter->drawPixmap(iconRect, icon, QRectF());
                break;
            }

            if (activeMeta == meta) {
                auto playingIcon = listview->getPlayPixmap();
                auto icon = QPixmap(playingIcon);
                auto centerF = QRectF(rect).center();
                auto iconRect = QRectF(centerF.x() - icon.width() / 2,
                                       centerF.y() - icon.height() / 2,
                                       icon.width(), icon.height());
                painter->drawPixmap(iconRect.toRect(), icon);

            } else {
                painter->setFont(font11);
                auto str = QString("%1").arg(index.row() + 1, rowCountSize, 10, QLatin1Char('0'));
                QFont font(font11);
                QFontMetrics fm(font);
                auto text = fm.elidedText(str, Qt::ElideMiddle, rect.width());
                painter->drawText(rect, flag, text);
            }
            break;
        }
        case Title: {
            painter->setPen(nameColor);
            painter->setFont(font14);
            QFont font(font14);
            QFontMetrics fm(font);
            auto text = fm.elidedText(meta->title, Qt::ElideMiddle, rect.width());
            painter->drawText(rect, flag, text);
            break;
        }
        case Length:
            painter->setPen(otherColor);
            painter->setFont(font11);
            painter->drawText(rect, flag, DMusic::lengthString(meta->length));
            break;
        default:
            break;
        }
    }
    painter->restore();
}

QSize MusicInfoItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    Q_D(const MusicInfoItemDelegate);

    auto listview = qobject_cast<const MusicListInfoView *>(option.widget);

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

QWidget *MusicInfoItemDelegate::createEditor(QWidget *parent,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const

{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void MusicInfoItemDelegate::setEditorData(QWidget *editor,
                                          const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}

void MusicInfoItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                         const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

MusicInfoItemDelegate::MusicInfoItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent), d_ptr(new MusicInfoItemDelegatePrivate(this))
{

}

MusicInfoItemDelegate::~MusicInfoItemDelegate()
{
}

void MusicInfoItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
//    Q_D(const MusicInfoItemDelegate);
    QStyledItemDelegate::initStyleOption(option, index);
    option->state = option->state & ~QStyle::State_HasFocus;
}
