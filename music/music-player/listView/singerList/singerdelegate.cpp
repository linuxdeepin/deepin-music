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

#include "singerdelegate.h"
#include "singerlistview.h"
#include "core/medialibrary.h"
#include "global.h"

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

void SingerDataDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    auto listview = qobject_cast<const SingerListView *>(option.widget);

    if (listview->viewMode() == QListView::IconMode) {
        drawIconMode(*painter, option, index);
    } else {
        drawListMode(*painter, option, index);
    }
}

QSize SingerDataDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    auto listview = qobject_cast<const SingerListView *>(option.widget);
    if (listview->viewMode() == QListView::IconMode) {
        return QSize(150, 150);
    } else {
        auto baseSize = QStyledItemDelegate::sizeHint(option, index);
        return QSize(baseSize.width(), 38);
    }
}

QWidget *SingerDataDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const

{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void SingerDataDelegate::setEditorData(QWidget *editor,
                                       const QModelIndex &index) const
{

    QStyledItemDelegate::setEditorData(editor, index);

}

void SingerDataDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

bool SingerDataDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    auto listview = qobject_cast<const SingerListView *>(option.widget);
    int borderWidth = 10;
    QRect rect = option.rect.adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth);
    if (index.isValid() && listview->viewMode() == QListView::IconMode && event->type() == QEvent::MouseButtonPress) {
        SingerInfo singertmp = index.data(Qt::UserRole).value<SingerInfo>();
        bool playFlag = singertmp.musicinfos.keys().contains(Player::instance()->activeMeta().hash);
        Player::PlaybackStatus playStatue = Player::instance()->status();
        if (playFlag) {
            if (playStatue == Player::Playing) {
                Player::instance()->pause();
            } else if (playStatue == Player::Paused) {
                Player::instance()->resume();
            }
        } else {
            QRect t_hoverRect(rect.x() + 50, rect.y() + 36, 50, 50);

            QPainterPath t_imageClipPath;
            t_imageClipPath.addEllipse(QRect(rect.x() + 50, rect.y() + 36, 50, 50));
            t_imageClipPath.closeSubpath();
            auto fillPolygon = t_imageClipPath.toFillPolygon();

            QMouseEvent *pressEvent = static_cast<QMouseEvent *>(event);
            QPointF pressPos = pressEvent->pos();

            if (fillPolygon.containsPoint(pressPos, Qt::OddEvenFill)) {
                if (singertmp.musicinfos.values().size() > 0) {
                    emit CommonService::getInstance()->setPlayModel(Player::RepeatAll);
                    Player::instance()->setCurrentPlayListHash("album", false);
                    Player::instance()->setPlayList(singertmp.musicinfos.values());
                    Player::instance()->playMeta(singertmp.musicinfos.values().first());
                    emit Player::instance()->signalPlayListChanged();
                }
            }
        }
        return false;
        return false;
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void SingerDataDelegate::drawIconMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto listview = qobject_cast<const SingerListView *>(option.widget);
    SingerInfo singertmp = index.data(Qt::UserRole).value<SingerInfo>();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    auto background = option.palette.background();
    painter.fillRect(option.rect, background);
    //绘制阴影
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

    bool playFlag = singertmp.musicinfos.keys().contains(Player::instance()->activeMeta().hash);
    Player::PlaybackStatus playStatue = Player::instance()->status();

    QColor fillColor(0, 0, 0);
    fillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        fillColor = "#000000";
        fillColor.setAlphaF(0.3);
    }
    int startHeight = rect.y() + rect.height() - 56;
    int fillAllHeight = 36;
    int curFillSize = fillAllHeight;
    QRect fillBlurRect(rect.x(), rect.y() + rect.height() - fillAllHeight, rect.width(), fillAllHeight);

    if (playFlag && (playStatue == Player::Playing)) {
        fillBlurRect = QRect(rect.x(), rect.y() + rect.height() - 70, rect.width(), 76);
        curFillSize = 70;
    }

    //设置模糊
    QImage t_image = opticon.pixmap(rect.width(), rect.height()).toImage();
    qreal t_ratio = t_image.devicePixelRatioF();
    curFillSize = static_cast<int>(curFillSize * t_ratio);

    t_image  = t_image.copy(0, rect.height() - curFillSize, t_image.width(), curFillSize);
    QTransform old_transform = painter.transform();
    painter.translate(fillBlurRect.topLeft());
    qt_blurImage(&painter, t_image, 35, false, false);
    painter.setTransform(old_transform);
    //设置模糊
    painter.fillRect(fillBlurRect, fillColor);

    //draw playing
    auto *listview2 = qobject_cast<SingerListView *>(const_cast<QWidget *>(option.widget));
    if (playFlag && playStatue == Player::Playing) {
        if (option.state & QStyle::State_MouseOver) {
            painter.drawPixmap(QRect(rect.x() + 56, rect.y() + 82, 36, 36), hoverSuspendImg);
        } else {
            if (listview2) {
                painter.drawPixmap(QRect(rect.x() + 64, rect.y() + 92, 22, 18), listview2->getPlayPixmap(true));
            }
        }
    }


    QRect fillRect(rect.x(), startHeight, rect.width(), fillAllHeight);

    QFont font = option.font;
    font.setPixelSize(14);
    painter.setFont(font);
    QFontMetrics fm(font);

    QRect nameFillRect(rect.x(), startHeight + 26, rect.width(), fillAllHeight / 2);
    nameFillRect.adjust(8, 0, -7, 0);
    auto nameText = fm.elidedText(singertmp.singerName.isEmpty() ? SingerListView::tr("Unknown artist") : singertmp.singerName, Qt::ElideMiddle, nameFillRect.width());
    painter.setPen(Qt::white);
    painter.drawText(nameFillRect, Qt::AlignLeft | Qt::AlignTop, nameText);

    font.setPixelSize(10);
    QFontMetrics extraNameFm(font);
    painter.setFont(font);
    QRect extraNameFillRect(rect.x(), startHeight + fillAllHeight / 2 + 1, rect.width(), fillAllHeight / 2);
    extraNameFillRect.adjust(8, 0, -7, 0);
    QBrush t_fillBrush(QColor(128, 128, 128, 0));

    fillColor.setAlphaF(0.3);
    if (listview->getThemeType() == 2) {
        fillColor = "#000000";
        fillColor.setAlphaF(0.3);
    }

    if (option.state & QStyle::State_Selected) {
        t_fillBrush = QBrush(QColor(128, 128, 128, 90));
    }

    if ((option.state & QStyle::State_MouseOver) && (!playFlag || (playFlag && playStatue == Player::Paused))) {
        QImage t_image = opticon.pixmap(rect.width(), rect.height()).toImage();
        int t_ratio = static_cast<int>(t_image.devicePixelRatioF());
        QRect t_imageRect(rect.width() / 2 - 25, rect.height() / 2 - 25, 50 * t_ratio, 50 * t_ratio);
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
    painter.fillRect(option.rect, t_fillBrush);
}

void SingerDataDelegate::drawListMode(QPainter &painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto listview = qobject_cast<const SingerListView *>(option.widget);
    auto singertmp = index.data(Qt::UserRole).value<SingerInfo>();

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
        //painter->drawRect(option.rect);
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
        QColor hovertColor(option.palette.shadow().color());
        if (option.state & QStyle::State_Selected)
            hovertColor.setAlphaF(0.2);
        painter.setBrush(hovertColor);
        QRect selecteColorRect = option.rect.adjusted(lrWidth, 0, -lrWidth, 0);
        painter.drawRoundedRect(selecteColorRect, 8, 8);
        painter.restore();
    }

    QColor nameColor("#090909"), otherColor("#797979");
    otherColor.setAlphaF(0.5);
    if (listview->getThemeType() == 2) {
        nameColor = QColor("#C0C6D4");
        otherColor = QColor("#C0C6D4");
        otherColor.setAlphaF(0.5);
    }

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


    QRect numRect(lrWidth + 10, option.rect.y() + 3, 32, 32);
    auto icon = option.icon;
    auto value = index.data(Qt::DecorationRole);
    if (value.type() == QVariant::Icon) {
        icon = qvariant_cast<QIcon>(value);
    }
    painter.save();
    QPainterPath clipPath;
    clipPath.addEllipse(numRect.adjusted(4, 4, -4, -4));
    painter.setClipPath(clipPath);
    painter.drawPixmap(numRect, icon.pixmap(numRect.width(), numRect.width()));
    painter.restore();

    //name
    painter.setPen(nameColor);
    QRect nameRect(60, option.rect.y(), w / 2 - 20, option.rect.height());
    painter.setFont(font14);
    auto nameText = songsFm.elidedText(singertmp.singerName.isEmpty() ? SingerListView::tr("Unknown artist") :
                                       singertmp.singerName, Qt::ElideMiddle, nameRect.width());
    painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

    painter.setPen(nameColor);

    //songs
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
    painter.setFont(font11);
    QFontMetrics measuringFont(font11);
    int strwidth = pixel2point(measuringFont.width(infoStr));
    QRect songsRect(option.rect.width() - tailwidth/*60 + w / 2 + w / 4*/, option.rect.y(), w / 4 - 20, option.rect.height());
    QString countText = songsFm.elidedText(infoStr, Qt::ElideMiddle, strwidth);
    painter.drawText(songsRect, Qt::AlignLeft | Qt::AlignVCenter, countText);
    painter.restore();
}

SingerDataDelegate::SingerDataDelegate(QWidget *parent): QStyledItemDelegate(parent)
{
    hoverPlayImg = DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/actions/play_hover_36px.svg");
    hoverSuspendImg = DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/actions/suspend_hover_36px.svg");
    shadowImg = DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/actions/shadow_176px.svg");
//    shadowImg = shadowImg.copy(5, 5, shadowImg.width() - 10, shadowImg.height() - 10);
}

SingerDataDelegate::~SingerDataDelegate()
{

}

void SingerDataDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
}

