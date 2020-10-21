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
#include <QEvent>
#include <QMouseEvent>
#include <DGuiApplicationHelper>

#include <DHiDPIHelper>

#include "../musiclistdataview.h"
#include "core/medialibrary.h"

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

const int PlayItemRightMargin = 20;

static inline int pixel2point(int pixel)
{
    return pixel * 96 / 72;
}

class MusicListDataDelegatePrivate
{
public:
    explicit MusicListDataDelegatePrivate(MusicListDataDelegate *parent): q_ptr(parent)
    {
        shadowImg = shadowImg.copy(5, 5, shadowImg.width() - 10, shadowImg.height() - 10);
    }

    QWidget *parentWidget = nullptr;
    QPixmap playing = DHiDPIHelper::loadNxPixmap(":/common/image/jumpto_playing_normal.svg");
//    QString playingIcon = ":/mpimage/light/music1.svg";
//    QString highlightPlayingIcon = ":/mpimage/light/music1.svg";
    QPixmap hoverPlayImg = DHiDPIHelper::loadNxPixmap(":/mpimage/dark/hover/play_hover.svg");
    QPixmap hoverSuspendImg = DHiDPIHelper::loadNxPixmap(":/mpimage/dark/hover/suspend_hover.svg");
    QPixmap shadowImg = DHiDPIHelper::loadNxPixmap(":/mpimage/light/shadow.svg");

    MusicListDataDelegate *q_ptr;
    bool containsMusic(QString uuid, MetaPtr playing, PlayMusicTypePtr currentList)const;
    Q_DECLARE_PUBLIC(MusicListDataDelegate)
};


bool MusicListDataDelegatePrivate::containsMusic(QString uuid, MetaPtr playing, PlayMusicTypePtr currentList)const
{

    if (uuid == AlbumResultListID || uuid == AlbumMusicListID) {
        if (playing->album == currentList->playlistMeta.metas.first()->album) {
            return true;
        }
    }
    if (uuid == ArtistResultListID || uuid == ArtistMusicListID) {
        if (playing->artist == currentList->playlistMeta.metas.first()->artist) {
            return true;
        }
    }
    return false;
}

void MusicListDataDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    Q_D(const MusicListDataDelegate);

    //QStyledItemDelegate::paint(painter, option, index);

    auto listview = qobject_cast<const MusicListDataView *>(option.widget);
    PlaylistPtr playlistPtr = listview->playlist();
    auto playMusicTypePtrList = listview->playMusicTypePtrList();
    if (index.row() >= playMusicTypePtrList.size()) {
        return;
    }
    auto PlayMusicTypePtr = playMusicTypePtrList[index.row()];

//    if (listview->playing() != nullptr && listview->playing()->album != PlayMusicTypePtr->name){
//        return
//    }

    if (listview->viewMode() == QListView::IconMode) {
        //QStyledItemDelegate::paint(painter, option, index);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::HighQualityAntialiasing);
        painter->setRenderHint(QPainter::SmoothPixmapTransform);

        auto background = option.palette.background();

        if (option.state & QStyle::State_Selected) {
//            background = option.palette.highlight();
        }

        painter->fillRect(option.rect, background);

        //draw shadow
//        int shadowBorderWidth = 10;
//        QRect shadowRect = option.rect.adjusted(shadowBorderWidth, shadowBorderWidth, -shadowBorderWidth, -shadowBorderWidth);;
//        QPainterPath roundRectShadowPath;
//        roundRectShadowPath.addRoundRect(shadowRect, 10, 10);
//        painter->save();
//        painter->setClipPath(roundRectShadowPath);
//        painter->drawPixmap(shadowRect, d->shadowImg);
//        painter->restore();

        //绘制阴影
        QRect shadowRect(option.rect.x() - 10, option.rect.y(), 158, 158);
        QPainterPath roundRectShadowPath;
        roundRectShadowPath.addRoundRect(shadowRect, 8, 8);
        painter->save();
        painter->setClipPath(roundRectShadowPath);
        painter->drawPixmap(shadowRect, d_ptr->shadowImg);
        painter->restore();

        int borderWidth = 0;
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


        //draw border
        painter->save();
        QColor borderPenColor("#000000");
        borderPenColor.setAlphaF(0.05);
        QPen borderPen(borderPenColor);
        borderPen.setWidthF(2);
        painter->setPen(borderPen);
        painter->drawRoundRect(rect/*.adjusted(1, 1, -1, 1)*/, 10, 10);
        painter->restore();

        bool playFlag = false;
        if (PlayMusicTypePtr->extraName.isEmpty()) {
            QColor fillColor(0, 0, 0);
            fillColor.setAlphaF(0.3);
            if (listview->getThemeType() == 2) {
                fillColor = "#000000";
                fillColor.setAlphaF(0.3);
            }

            if (listview->playing() != nullptr
                    && listview->playingState()
                    && d->containsMusic(listview->playlist()->id(), listview->playing(), PlayMusicTypePtr)) {
                playFlag = true;
            }

            QRect fillBlurRect(rect.x(), rect.y() + rect.height() - 36, rect.width(), 36);
            int curFillSize = 36;
            if (playFlag) {
                fillBlurRect = QRect(rect.x(), rect.y() + rect.height() - 64, rect.width(), 70);
                curFillSize = 64;
            }
            //设置模糊
            QImage t_image = icon.pixmap(rect.width(), rect.height()).toImage();
            qreal t_ratio = t_image.devicePixelRatioF();
            curFillSize = static_cast<int>( curFillSize * t_ratio);

            t_image  = t_image.copy(0, rect.height() - curFillSize, t_image.width(), curFillSize);
            QTransform old_transform = painter->transform();
            painter->translate(fillBlurRect.topLeft());
            qt_blurImage(painter, t_image, 35, false, false);
            painter->setTransform(old_transform);
            //设置模糊
            painter->fillRect(fillBlurRect, fillColor);

            //draw playing
            if (playFlag) {
                if (option.state & QStyle::State_MouseOver) {
                    painter->drawPixmap(QRect(rect.x() + 56, rect.y() + 82, 36, 36), d->hoverSuspendImg);
                } else {
                    painter->drawPixmap(QRect(rect.x() + 64, rect.y() + 92, 22, 18), listview->getAlbumPixmap());
                }
            }

            QRect fillRect(rect.x(), rect.y() + rect.height() - 36, rect.width(), 36);
            QFont font = option.font;
            font.setFamily("SourceHanSansSC");
            font.setWeight(QFont::Normal);
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
            if (listview->getThemeType() == 2) {
                fillColor = "#000000";
                fillColor.setAlphaF(0.3);
            }
            int startHeight = rect.y() + rect.height() - 46;
            int fillAllHeight = 46;
            int curFillSize = fillAllHeight;
            QRect fillBlurRect(rect.x(), rect.y() + rect.height() - fillAllHeight, rect.width(), fillAllHeight);

            if (listview->playing() != nullptr
                    && listview->playingState()
                    && d->containsMusic(listview->playlist()->id(), listview->playing(), PlayMusicTypePtr)) {
                playFlag = true;
            }

            if (playFlag) {
                fillBlurRect = QRect(rect.x(), rect.y() + rect.height() - 80, rect.width(), 86);
                curFillSize = 80;
            }

            //设置模糊
            QImage t_image = icon.pixmap(rect.width(), rect.height()).toImage();
            qreal t_ratio = t_image.devicePixelRatioF();
            curFillSize = static_cast<int>( curFillSize * t_ratio);

            t_image  = t_image.copy(0, rect.height() - curFillSize, t_image.width(), curFillSize);
            QTransform old_transform = painter->transform();
            painter->translate(fillBlurRect.topLeft());
            qt_blurImage(painter, t_image, 35, false, false);
            painter->setTransform(old_transform);
            //设置模糊
            painter->fillRect(fillBlurRect, fillColor);

            //draw playing
            if (playFlag) {
                if (option.state & QStyle::State_MouseOver) {
                    painter->drawPixmap(QRect(rect.x() + 56, rect.y() + 82, 36, 36), d->hoverSuspendImg);
                } else {
                    painter->drawPixmap(QRect(rect.x() + 64, rect.y() + 92, 22, 18), listview->getAlbumPixmap());
                }
            }

            QRect fillRect(rect.x(), startHeight, rect.width(), fillAllHeight);

            QFont font = option.font;
            font.setPixelSize(14);
            painter->setFont(font);
            QFontMetrics fm(font);

            QRect nameFillRect(rect.x(), startHeight + 2, rect.width(), fillAllHeight / 2);
            nameFillRect.adjust(8, 0, -7, 0);
            auto nameText = fm.elidedText(PlayMusicTypePtr->name, Qt::ElideMiddle, nameFillRect.width());
            painter->setPen(Qt::white);
            painter->drawText(nameFillRect, Qt::AlignLeft | Qt::AlignTop, nameText);

            font.setPixelSize(10);
            QFontMetrics extraNameFm(font);
            painter->setFont(font);
            QRect extraNameFillRect(rect.x(), startHeight + fillAllHeight / 2 + 1, rect.width(), fillAllHeight / 2);
            extraNameFillRect.adjust(8, 0, -7, 0);
            auto extraNameText = extraNameFm.elidedText(PlayMusicTypePtr->extraName, Qt::ElideMiddle, extraNameFillRect.width());
            painter->setPen(Qt::white);
            painter->drawText(extraNameFillRect, Qt::AlignLeft | Qt::AlignTop, extraNameText);
        }


        QBrush t_fillBrush(QColor(128, 128, 128, 0));

        QColor fillColor(0, 0, 0);
        fillColor.setAlphaF(0.3);
        if (listview->getThemeType() == 2) {
            fillColor = "#000000";
            fillColor.setAlphaF(0.3);
        }

        if (option.state & QStyle::State_Selected) {
            t_fillBrush = QBrush(QColor(128, 128, 128, 90));
        }

        if ((option.state & QStyle::State_MouseOver) && !playFlag) {
            if (!playlistPtr->playingStatus() || !playFlag ) {
                QImage t_image = icon.pixmap(rect.width(), rect.height()).toImage();
                int t_ratio = static_cast<int>(t_image.devicePixelRatioF());
                QRect t_imageRect(rect.width() / 2 - 25, rect.height() / 2 - 25, 50 * t_ratio, 50 * t_ratio);
                t_image  = t_image.copy(t_imageRect);
                QRect t_hoverRect(rect.x() + 50, rect.y() + 36, 50 * t_ratio, 50 * t_ratio);

                QTransform old_transform = painter->transform();
                painter->translate(t_hoverRect.topLeft());

                QPainterPath t_imageClipPath;
                t_imageClipPath.addEllipse(QRect(0, 0, 50, 50));
                painter->setClipPath(t_imageClipPath);

                qt_blurImage(painter, t_image, 30, false, false);
                painter->setTransform(old_transform);
                painter->fillRect(t_hoverRect, fillColor);

                QPixmap t_hoverPlayImg(d->hoverPlayImg);
                t_hoverPlayImg.setDevicePixelRatio(option.widget->devicePixelRatioF());
                //            t_hoverRect.adjust(0, 0, -7 * t_ratio, -7 * t_ratio);
                QRect t_pixMapRect(rect.x() + 53, rect.y() + 40, 43, 43);
                QIcon icon(t_hoverPlayImg);
                painter->drawPixmap(t_pixMapRect, t_hoverPlayImg);
            } /*else {
                QPixmap t_hoverPlayImg(d->hoverSuspendImg);
                t_hoverPlayImg.setDevicePixelRatio(option.widget->devicePixelRatioF());
                //            t_hoverRect.adjust(0, 0, -7 * t_ratio, -7 * t_ratio);
                QRect t_pixMapRect(rect.x() + 53, rect.y() + 40, 43, 43);
                QIcon icon(t_hoverPlayImg);
                painter->drawPixmap(t_pixMapRect, t_hoverPlayImg);
            }*/
        }

        painter->fillRect(option.rect, t_fillBrush);

        painter->restore();

    } else {
        painter->save();
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

        auto background = (index.row() % 2) == 1 ? baseColor : alternateBaseColor;
        //auto background = baseColor;

        int lrWidth = 10;
        if (!(option.state & QStyle::State_Selected) && !(option.state & QStyle::State_MouseOver) ) {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(background);
            //painter->drawRect(option.rect);
            QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
            painter->drawRoundedRect(selecteColorRect, 8, 8);
            painter->restore();
            //painter->fillRect(option.rect, background);
        }

        if (option.state & QStyle::State_Selected) {
            painter->save();
            painter->setPen(Qt::NoPen);
//            QColor selectColor("#000000");
//            if (listview->getThemeType() == 2) {
//                selectColor = QColor("#FFFFFF");
//            }
//            selectColor.setAlphaF(0.2);
            QColor selectColor(option.palette.highlight().color());
            painter->setBrush(selectColor);
            QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
            painter->drawRoundedRect(selecteColorRect, 8, 8);
            painter->restore();
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

        QColor nameColor("#090909"), otherColor("#797979");
        otherColor.setAlphaF(0.5);
        if (listview->getThemeType() == 2) {
            nameColor = QColor("#C0C6D4");
            otherColor = QColor("#C0C6D4");
            otherColor.setAlphaF(0.5);
        }

        if (PlayMusicTypePtr->extraName.isEmpty()) {
            //QStyledItemDelegate::paint(painter, option, index);

            QFont font11 = option.font;
            font11.setFamily("SourceHanSansSC");
            font11.setWeight(QFont::Normal);
            font11.setPixelSize(11);
            QFont font14 = option.font;
            font14.setFamily("SourceHanSansSC");
            font14.setWeight(QFont::Normal);
            font14.setPixelSize(14);

            QFontMetrics songsFm(font14);
            auto tailwidth = pixel2point(songsFm.width("0000-00-00")) + PlayItemRightMargin  + 20;
            auto w = option.rect.width() - 0 - tailwidth;

            bool playingFlag = false;
            if (listview->playing() != nullptr && (listview->playing()->artist == PlayMusicTypePtr->name
                                                   || (listview->playing()->artist.isEmpty() && !PlayMusicTypePtr->playlistMeta.metas.isEmpty()
                                                       && PlayMusicTypePtr->playlistMeta.metas.begin().value()->artist.isEmpty()))) {
                if (option.state & QStyle::State_Selected) {
                    nameColor = option.palette.highlightedText().color();
                    otherColor = option.palette.highlightedText().color();
                } else {
                    //nameColor = QColor("#2CA7F8");
                    nameColor = QColor(DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
                    otherColor = QColor("#2CA7F8");
                }
                font14.setFamily("SourceHanSansSC");
                font14.setWeight(QFont::Medium);
                playingFlag = true;
            } else {
                if (option.state & QStyle::State_Selected) {
                    nameColor = option.palette.highlightedText().color();
                    otherColor = option.palette.highlightedText().color();
                }
            }
            //icon
            if (playlistPtr->searchStr().isEmpty()) {
                if (!playingFlag) {
                    QRect numRect(lrWidth + 10, option.rect.y() + 3, 32, 32);
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
                } else {
                    QRect numRect(lrWidth + 10, option.rect.y(), 40, option.rect.height());
                    auto icon = listview->getPlayPixmap();
                    if (option.state & QStyle::State_Selected) {
                        icon = listview->getSidebarPixmap();
                    }
                    qreal t_ratio = icon.devicePixelRatioF();
                    auto centerF = numRect.center();
                    QRect t_ratioRect;
                    t_ratioRect.setX(0);
                    t_ratioRect.setY(0);
                    t_ratioRect.setWidth(static_cast<int>(icon.width() / t_ratio));
                    t_ratioRect.setHeight(static_cast<int>(icon.height() / t_ratio));
                    auto iconRect = QRectF(centerF.x() - t_ratioRect.width() / 2,
                                           centerF.y() - t_ratioRect.height() / 2,
                                           t_ratioRect.width(), t_ratioRect.height());
                    painter->drawPixmap(iconRect.toRect(), icon);
                }

            } else {
                int rowCount = listview->model()->rowCount();
                auto rowCountSize = QString::number(rowCount).size();
                rowCountSize = qMax(rowCountSize, 2);
                QFont measuringFont(font11);
                painter->setPen(otherColor);
                QRect numRect(lrWidth + 10, option.rect.y(), 40, option.rect.height());
                painter->setFont(font11);
                auto str = QString("%1").arg(index.row() + 1, rowCountSize, 10, QLatin1Char('0'));
                QFont font(font11);
                QFontMetrics fm(font);
                auto text = fm.elidedText(str, Qt::ElideMiddle, numRect.width());
                painter->drawText(numRect, Qt::AlignCenter, text);
            }


            painter->setPen(nameColor);
            //name
            QRect nameRect(59, option.rect.y(), w / 2 - 20, option.rect.height());
            painter->setFont(font14);
            auto nameText = songsFm.elidedText(PlayMusicTypePtr->name, Qt::ElideMiddle, nameRect.width());
            painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

            int sortMetasSize = PlayMusicTypePtr->playlistMeta.sortMetas.size();
            QString infoStr;
            if (sortMetasSize == 0) {
                infoStr = QString("   ") + tr("No songs");
            } else if (sortMetasSize == 1) {
                infoStr = QString("   ") + tr("1 song");
            } else {
                infoStr = QString("   ") + tr("%1 songs").arg(sortMetasSize);
            }

            painter->setPen(nameColor);
            painter->setFont(font11);
            QRect rect(w, option.rect.y(), tailwidth - 20, option.rect.height());
            painter->drawText(rect, Qt::AlignRight | Qt::AlignVCenter, infoStr);

        } else {
            QFont font11 = option.font;
            font11.setFamily("SourceHanSansSC");
            font11.setWeight(QFont::Normal);
            font11.setPixelSize(11);
            QFont font14 = option.font;
            font14.setFamily("SourceHanSansSC");
            font14.setWeight(QFont::Normal);
            font14.setPixelSize(14);

            int rowCount = listview->model()->rowCount();
            auto rowCountSize = QString::number(rowCount).size();
            rowCountSize = qMax(rowCountSize, 2);

            QFontMetrics songsFm(font11);
            auto tailwidth = pixel2point(songsFm.width("0000-00-00")) + PlayItemRightMargin  + 20;
            auto w = option.rect.width() - 0 - tailwidth;

            //num
            if (listview->playing() != nullptr
                    && listview->playingState()
                    && (listview->playing()->album == PlayMusicTypePtr->name
                        || (listview->playing()->album.isEmpty() && !PlayMusicTypePtr->playlistMeta.metas.isEmpty()
                            && PlayMusicTypePtr->playlistMeta.metas.begin().value()->album.isEmpty()))) {
                if (option.state & QStyle::State_Selected) {
                    nameColor = option.palette.highlightedText().color();
                    otherColor = option.palette.highlightedText().color();
                } else {
                    nameColor = QColor(DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
                    otherColor = QColor("#2CA7F8");
                }
                font14.setFamily("SourceHanSansSC");
                font14.setWeight(QFont::Medium);
                QRect numRect(lrWidth, option.rect.y(), 40, option.rect.height());
                auto icon = listview->getPlayPixmap();
                if (option.state & QStyle::State_Selected) {
                    icon = listview->getSidebarPixmap();
                }
                qreal t_ratio = icon.devicePixelRatioF();
                auto centerF = numRect.center();
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
                if (option.state & QStyle::State_Selected) {
                    nameColor = option.palette.highlightedText().color();
                    otherColor = option.palette.highlightedText().color();
                }

                painter->setPen(otherColor);
                QRect numRect(lrWidth, option.rect.y(), 40, option.rect.height());
                painter->setFont(font11);
                auto str = QString("%1").arg(index.row() + 1, rowCountSize, 10, QLatin1Char('0'));
                QFont font(font11);
                QFontMetrics fm(font);
                auto text = fm.elidedText(str, Qt::ElideMiddle, numRect.width());
                painter->drawText(numRect, Qt::AlignCenter, text);
            }


            //name
            painter->setPen(nameColor);
            QRect nameRect(50, option.rect.y(), w / 2 - 20, option.rect.height());
            painter->setFont(font14);
            auto nameText = songsFm.elidedText(PlayMusicTypePtr->name, Qt::ElideMiddle, nameRect.width());
            painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

            painter->setPen(nameColor);
            //extraname
            QRect extraRect(50 + w / 2, option.rect.y(), w / 4 - 20, option.rect.height());
            painter->setFont(font11);
            auto extraText = songsFm.elidedText(PlayMusicTypePtr->extraName, Qt::ElideMiddle, extraRect.width());
            painter->drawText(extraRect, Qt::AlignLeft | Qt::AlignVCenter, extraText);

            //songs
            int sortMetasSize = PlayMusicTypePtr->playlistMeta.sortMetas.size();
            QString infoStr;
            if (sortMetasSize == 0) {
                infoStr = QString("   ") + tr("No songs");
            } else if (sortMetasSize == 1) {
                infoStr = QString("   ") + tr("1 song");
            } else {
                infoStr = QString("   ") + tr("%1 songs").arg(sortMetasSize);
            }
            painter->save();
            QFont measuringFont(font11);
            QRect songsRect(50 + w / 2 + w / 4, option.rect.y(), w / 4 - 20, option.rect.height());
            painter->drawText(songsRect, Qt::AlignLeft | Qt::AlignVCenter, infoStr);
            painter->restore();

            //day
            QRect dayRect(w, option.rect.y(), tailwidth - 20, option.rect.height());
            painter->setFont(font11);
            QString dayStr = QDateTime::fromMSecsSinceEpoch(PlayMusicTypePtr->timestamp / static_cast<qint64>(1000)).toString("yyyy-MM-dd");
            painter->drawText(dayRect, Qt::AlignRight | Qt::AlignVCenter, dayStr);
        }

        painter->restore();
    }
}

QSize MusicListDataDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    auto listview = qobject_cast<const MusicListDataView *>(option.widget);
    if (listview->viewMode() == QListView::IconMode) {
        return QSize(150, 150);
    }
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

bool MusicListDataDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    auto listview = qobject_cast<const MusicListDataView *>(option.widget);
    int borderWidth = 10;
    QRect rect = option.rect.adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth);
    if (index.isValid() && listview->viewMode() == QListView::IconMode && event->type() == QEvent::MouseButtonPress) {
        if (!listview->playingState()) {
            QRect t_hoverRect(rect.x() + 50, rect.y() + 36, 50, 50);

            QPainterPath t_imageClipPath;
            t_imageClipPath.addEllipse(QRect(rect.x() + 50, rect.y() + 36, 50, 50));
            t_imageClipPath.closeSubpath();
            auto fillPolygon = t_imageClipPath.toFillPolygon();

            QMouseEvent *pressEvent = static_cast<QMouseEvent *>(event);
            QPointF pressPos = pressEvent->pos();

            if (fillPolygon.containsPoint(pressPos, Qt::OddEvenFill))
                Q_EMIT hoverPress(index);
        } else {
            QRect t_hoverRect (rect.x() + 64, rect.y() + 96, 22, 18);
            QPainterPath t_imageClipPath;
            t_imageClipPath.addEllipse(QRect(rect.x() + 64, rect.y() + 96, 25, 25));
            t_imageClipPath.closeSubpath();
            auto fillPolygon = t_imageClipPath.toFillPolygon();

            //QMouseEvent *pressEvent = static_cast<QMouseEvent *>(event);
            //QPointF pressPos = pressEvent->pos();
            Q_EMIT hoverPress(index);
        }
        return false;
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
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

