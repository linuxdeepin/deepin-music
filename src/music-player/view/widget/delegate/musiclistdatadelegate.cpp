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

#include "musiclistdatadelegate.h"

#include <QPainter>
#include <QDebug>
#include <QDate>

#include <DHiDPIHelper>

#include "../musiclistdataview.h"

const int PlayItemRightMargin = 20;

static inline int pixel2point(int pixel)
{
    return pixel * 96 / 72;
}

class MusicListDataDelegatePrivate
{
public:
    explicit MusicListDataDelegatePrivate(MusicListDataDelegate *parent): q_ptr(parent) {}

    QWidget *parentWidget;
    QPixmap playing = DHiDPIHelper::loadNxPixmap(":/common/image/jumpto_playing_normal.svg");

    MusicListDataDelegate *q_ptr;
    Q_DECLARE_PUBLIC(MusicListDataDelegate)
};

void MusicListDataDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    Q_D(const MusicListDataDelegate);

    //QStyledItemDelegate::paint(painter, option, index);

    auto listview = qobject_cast<const MusicListDataView *>(option.widget);
    PlaylistPtr playlistPtr = listview->playlist();
    auto playMusicTypePtrList = playlistPtr->playMusicTypePtrList();
    if (index.row() >= playMusicTypePtrList.size()) {
        return;
    }
    auto PlayMusicTypePtr = playMusicTypePtrList[index.row()];

    if (listview->viewMode() == QListView::IconMode) {
        //QStyledItemDelegate::paint(painter, option, index);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::HighQualityAntialiasing);

        auto background = option.palette.background();

        if (option.state & QStyle::State_Selected) {
            //background = option.palette.highlight();
        }

        //painter->fillRect(option.rect, background);

        int borderWidth = 10;
        QRect rect = option.rect.adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth);
        QPainterPath roundRectPath;
        roundRectPath.addRoundRect(rect, 10, 10);
        painter->setClipPath(roundRectPath);

        auto icon = option.icon;
        auto value = index.data(Qt::DecorationRole);
        if (value.type() == QVariant::Icon) {
            icon = qvariant_cast<QIcon>(value);
        }
        painter->drawPixmap(rect, icon.pixmap(rect.width(), rect.width()));

        if (PlayMusicTypePtr->extraName.isEmpty()) {
            QColor fillColor(0, 0, 0);
            fillColor.setAlphaF(0.3);
            //draw playing
            if (listview->playing() != nullptr && listview->playing()->artist == PlayMusicTypePtr->name) {
                QRect fillPlayingRect(rect.x(), rect.y() + rect.height() - 64, rect.width(), 28);
                painter->fillRect(fillPlayingRect, fillColor);

                painter->drawPixmap(QRect(rect.x() + 64, rect.y() + 96, 22, 18), d->playing);
            }

            QRect fillRect(rect.x(), rect.y() + rect.height() - 36, rect.width(), 36);
            painter->fillRect(fillRect, fillColor);

            QFont font = option.font;
            font.setFamily("SourceHanSansSC-Normal");
            font.setPixelSize(14);
            painter->setFont(font);
            QFontMetrics fm(font);
            fillRect.adjust(8, 0, -7, 0);
            auto text = fm.elidedText(PlayMusicTypePtr->name, Qt::ElideMiddle, fillRect.width());
            painter->setPen(Qt::white);
            painter->drawText(fillRect, Qt::AlignLeft | Qt::AlignVCenter, text);
        } else {
            QColor fillColor(0, 0, 0);
            fillColor.setAlphaF(0.3);

            //draw playing
            if (listview->playing() != nullptr && listview->playing()->album == PlayMusicTypePtr->name) {
                QRect fillPlayingRect(rect.x(), rect.y() + rect.height() - 83, rect.width(), 39);
                painter->fillRect(fillPlayingRect, fillColor);

                painter->drawPixmap(QRect(rect.x() + 64, rect.y() + 82, 22, 18), d->playing);
            }

            int startHeight = rect.y() + rect.height() - 44;
            int fillAllHeight = 44;

            QRect fillRect(rect.x(), startHeight, rect.width(), fillAllHeight);
            painter->fillRect(fillRect, fillColor);

            QFont font = option.font;
            font.setPixelSize(14);
            painter->setFont(font);
            QFontMetrics fm(font);

            QRect nameFillRect(rect.x(), startHeight, rect.width(), fillAllHeight / 2);
            nameFillRect.adjust(8, 0, -7, 0);
            auto nameText = fm.elidedText(PlayMusicTypePtr->name, Qt::ElideMiddle, fillRect.width());
            painter->setPen(Qt::white);
            painter->drawText(nameFillRect, Qt::AlignLeft | Qt::AlignTop, nameText);

            font.setPixelSize(11);
            painter->setFont(font);
            QRect extraNameFillRect(rect.x(), startHeight + fillAllHeight / 2 - 5, rect.width(), fillAllHeight / 2);
            extraNameFillRect.adjust(8, 0, -7, 0);
            auto extraNameText = fm.elidedText(PlayMusicTypePtr->extraName, Qt::ElideMiddle, fillRect.width());
            painter->setPen(Qt::white);
            painter->drawText(extraNameFillRect, Qt::AlignLeft | Qt::AlignTop, extraNameText);
        }


        QBrush t_fillBrush(QColor(128, 128, 128, 0));
        if (option.state & QStyle::State_Selected) {
            t_fillBrush = QBrush(QColor(128, 128, 128, 90));
        }
        painter->fillRect(option.rect, t_fillBrush);

        painter->restore();

    } else {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::HighQualityAntialiasing);

        auto background = (index.row() % 2) == 0 ? option.palette.base() : option.palette.alternateBase();

        if (option.state & QStyle::State_Selected) {
            background = option.palette.dark();
        }
        painter->fillRect(option.rect, background);

        if (PlayMusicTypePtr->extraName.isEmpty()) {
            //QStyledItemDelegate::paint(painter, option, index);

            QFont font11 = option.font;
            font11.setPixelSize(11);
            QFont font12 = option.font;
            font12.setPixelSize(12);

            QFontMetrics songsFm(font12);
            auto tailwidth = pixel2point(songsFm.width("0000-00-00")) + PlayItemRightMargin  + 20;
            auto w = option.rect.width() - 0 - tailwidth;

            //icon
            QRect numRect(0, option.rect.y(), 40, option.rect.height());
            auto icon = option.icon;
            auto value = index.data(Qt::DecorationRole);
            if (value.type() == QVariant::Icon) {
                icon = qvariant_cast<QIcon>(value);
            }
            painter->save();
            QPainterPath clipPath;
            clipPath.addEllipse(numRect.adjusted(4, 4, -4, -4));
            painter->setClipPath(clipPath);
            painter->drawPixmap(numRect, icon.pixmap(numRect.width(), numRect.width()));
            painter->restore();

            //name
            QRect nameRect(40, option.rect.y(), w / 2 - 20, option.rect.height());
            painter->setFont(font12);
            auto nameText = songsFm.elidedText(PlayMusicTypePtr->name, Qt::ElideMiddle, nameRect.width());
            painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

            int sortMetasSize = PlayMusicTypePtr->playlistMeta.sortMetas.size();
            QString infoStr;
            if (sortMetasSize == 0) {
                infoStr = tr("   No songs");
            } else if (sortMetasSize == 1) {
                infoStr = tr("   1 song");
            } else {
                infoStr = tr("   %1 songs").arg(sortMetasSize);
            }

            QFont measuringFont(option.font);
            QRect rect(w, option.rect.y(), tailwidth - 20, option.rect.height());
            painter->drawText(rect, Qt::AlignRight | Qt::AlignVCenter, infoStr);

        } else {
            QFont font11 = option.font;
            font11.setPixelSize(11);
            QFont font12 = option.font;
            font12.setPixelSize(12);

            int rowCount = listview->model()->rowCount();
            auto rowCountSize = QString::number(rowCount).size();
            rowCountSize = qMax(rowCountSize, 2);

            QFontMetrics songsFm(font12);
            auto tailwidth = pixel2point(songsFm.width("0000-00-00")) + PlayItemRightMargin  + 20;
            auto w = option.rect.width() - 0 - tailwidth;

            //num
            QRect numRect(0, option.rect.y(), 40, option.rect.height());
            painter->setFont(font11);
            auto str = QString("%1").arg(index.row() + 1, rowCountSize, 10, QLatin1Char('0'));
            QFont font(font11);
            QFontMetrics fm(font);
            auto text = fm.elidedText(str, Qt::ElideMiddle, numRect.width());
            painter->drawText(numRect, Qt::AlignCenter, text);

            //name
            QRect nameRect(40, option.rect.y(), w / 2 - 20, option.rect.height());
            painter->setFont(font12);
            auto nameText = songsFm.elidedText(PlayMusicTypePtr->name, Qt::ElideMiddle, nameRect.width());
            painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

            //extraname
            QRect extraRect(40 + w / 2, option.rect.y(), w / 4 - 20, option.rect.height());
            painter->setFont(font12);
            auto extraText = songsFm.elidedText(PlayMusicTypePtr->extraName, Qt::ElideMiddle, extraRect.width());
            painter->drawText(extraRect, Qt::AlignLeft | Qt::AlignVCenter, extraText);

            //songs
            int sortMetasSize = PlayMusicTypePtr->playlistMeta.sortMetas.size();
            QString infoStr;
            if (sortMetasSize == 0) {
                infoStr = tr("   No songs");
            } else if (sortMetasSize == 1) {
                infoStr = tr("   1 song");
            } else {
                infoStr = tr("   %1 songs").arg(sortMetasSize);
            }
            painter->save();
            QFont measuringFont(font12);
            QRect songsRect(40 + w / 2 + w / 4, option.rect.y(), w / 4 - 20, option.rect.height());
            painter->drawText(songsRect, Qt::AlignLeft | Qt::AlignVCenter, infoStr);
            painter->restore();

            //day
            QRect dayRect(w, option.rect.y(), tailwidth - 20, option.rect.height());
            painter->setFont(font12);
            QString dayStr = QDateTime::fromMSecsSinceEpoch(PlayMusicTypePtr->timestamp / (qint64)1000).toString("yyyy-MM-dd");
            painter->drawText(dayRect, Qt::AlignRight | Qt::AlignVCenter, dayStr);
        }

        painter->restore();
    }
}

QSize MusicListDataDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *MusicListDataDelegate::createEditor(QWidget *parent,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const

{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void MusicListDataDelegate::setEditorData(QWidget *editor,
                                          const QModelIndex &index) const
{

    QStyledItemDelegate::setEditorData(editor, index);

}

void MusicListDataDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                         const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

MusicListDataDelegate::MusicListDataDelegate(QWidget *parent): QStyledItemDelegate(parent), d_ptr(new MusicListDataDelegatePrivate(this))
{
    Q_D(MusicListDataDelegate);
    d->parentWidget = parent;
    d->playing = d->playing.scaled(22, 18);
}

MusicListDataDelegate::~MusicListDataDelegate()
{

}

void MusicListDataDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
}
