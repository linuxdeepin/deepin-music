/*
 * Copyright (C) 2020 chengdu Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <zhaoyongming@uniontech.com>
 *
 * Maintainer: Iceyer <zhaoyongming@uniontech.com>
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
#include "musicsearchlistdelegate.h"
#include "../musicsearchlistview.h"
#include "util/pinyinsearch.h"

#include <DHiDPIHelper>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QTextDocument>
#include <QStyleOptionViewItem>
#include <QTextCursor>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
using namespace DMusic;
using namespace DMusic;
QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

//static inline int pixel2point(int pixel)
//{
//    return pixel * 96 / 72;
//}

//const int PlayItemRightMargin = 20;

class MusicSearchListDelegatePrivate
{
public:
    explicit MusicSearchListDelegatePrivate(MusicSearchListDelegate *parent)
        : q_ptr(parent)
    {

    }

    QString playingIcon = ":/images/logo.svg";
    QString highlightPlayingIcon = ":/images/logo.svg";
    QPixmap shadowImg = DHiDPIHelper::loadNxPixmap(":/mpimage/light/shadow.svg");
    MusicSearchListDelegate *q_ptr;
    Q_DECLARE_PUBLIC(MusicSearchListDelegate)
};

MusicSearchListDelegate::MusicSearchListDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
//    , d_ptr(new MusicSearchListDelegatePrivate(this))
{

}

MusicSearchListDelegate::~MusicSearchListDelegate()
{

}

void MusicSearchListDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
}

bool MusicSearchListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(event)
    Q_UNUSED(model)
    Q_UNUSED(option)
    Q_UNUSED(index)
    return true;
}

void MusicSearchListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //Q_D(const MusicSearchListDelegate);

    /********************设置基础颜色***************************/
    QColor backColor("#FFFFFF");
    backColor.setAlphaF(0.2);

    //获取当前行信息
    auto listview = qobject_cast<const MusicSearchListview *>(option.widget);
    PlaylistPtr playlistPtr = listview->playlist();
    QString searchText = listview->getSearchText();
    if (playlistPtr == nullptr) {
        return;
    }
    auto playMusicTypePtrList = listview->playMusicTypePtrList();
    auto metaPtrList = listview->playMetaList();
    if (playlistPtr->id() == MusicCandListID) {
        if (index.row() >= metaPtrList.size()) {
            return;
        }
    } else {
        if (index.row() >= playMusicTypePtrList.size()) {
            return;
        }
    }

    QColor textColor;
    QColor lightColor;

    //主题改变需要修改color
    if (listview->getThemeType() == 2) {
        textColor = QColor("#FFFFFF");
    } else {
        textColor = QColor("#000000");
    }

    DPalette pa = option.palette;
    QBrush selectBrush = pa.brush(QPalette::Active, DPalette:: Highlight);
    QColor selectColor = selectBrush.color();
    lightColor = selectColor;

    //绘制选中状态
    if (option.state & QStyle::State_MouseOver) {
        // Q_EMIT this->SearchClear();

        painter->save();
        painter->setPen(Qt::NoPen);
        QColor hovertColor(option.palette.highlight().color());

        textColor = QColor("#FFFFFF");
        lightColor = QColor("#FFFFFF");

        if (option.state & QStyle::State_Selected)
            hovertColor.setAlphaF(0.2);
        painter->setBrush(hovertColor);
        QRect selecteColorRect = option.rect.adjusted(0, 0, 0, 0);
        painter->drawRoundedRect(selecteColorRect, 0, 0);
        painter->restore();

        emit SearchClear();
    }
    //绘制上下键选中
    if (index.row() == listview->getIndexInt()
            && listview->getIndexInt() >= 0) {
        painter->save();
        painter->setPen(Qt::NoPen);
        QColor hovertColor(option.palette.highlight().color());
//        lightColor = option.palette.highlightedText().color();
        textColor = QColor("#FFFFFF");
        lightColor = QColor("#FFFFFF");

        if (option.state & QStyle::State_Selected)
            hovertColor.setAlphaF(0.2);
        painter->setBrush(hovertColor);
        QRect selecteColorRect = option.rect.adjusted(0, 0, 0, 0);
        painter->drawRoundedRect(selecteColorRect, 8, 8);
        painter->restore();
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);

    /***********************设置字体***************************/
    QFont textFont = option.font;
    textFont.setFamily("SourceHanSansSC");
    textFont.setWeight(QFont::Normal);
    textFont.setPixelSize(14);

    if (playlistPtr->id() == MusicCandListID) {
        auto metaPtr = metaPtrList[index.row()];

        /***********************绘制文字***************************/
        QString mtext;
        if (metaPtr->artist.size() == 0) {
            mtext = metaPtr->title;
        } else {
            mtext = metaPtr->title + " - " + metaPtr->artist;
        }
        QFontMetricsF fontWidth(textFont);
        mtext = fontWidth.elidedText(mtext, Qt::ElideMiddle, 280);

        QStyleOptionViewItem viewOption(option);
        initStyleOption(&viewOption, index);
        if (option.state.testFlag(QStyle::State_HasFocus))
            viewOption.state = viewOption.state ^ QStyle::State_HasFocus;
        QStyle *pStyle = viewOption.widget ? viewOption.widget->style() : QApplication::style();
        viewOption.text = "";
        pStyle->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);


        QTextDocument document;
        document.setPlainText(mtext);
        bool found = false;
        QTextCursor highlight_cursor(&document);
        QTextCursor cursor(&document);

        cursor.beginEditBlock();
        QTextCharFormat color_format(highlight_cursor.charFormat());
        color_format.setForeground(lightColor);

        /*-------------LineUnderCursor-------------*/
        QTextCursor testcursor(&document);
        testcursor.select(QTextCursor::LineUnderCursor);
        QTextCharFormat fmt;
        fmt.setForeground(textColor);
        testcursor.mergeCharFormat(fmt);
        testcursor.clearSelection();
        testcursor.movePosition(QTextCursor::EndOfLine);

        while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
            highlight_cursor = document.find(searchText, highlight_cursor);
            if (!highlight_cursor.isNull()) {
                if (!found)
                    found = true;
                highlight_cursor.mergeCharFormat(color_format);
            }
        }
        cursor.endEditBlock();

        QAbstractTextDocumentLayout::PaintContext paintContext;
        QRect textRect(32, option.rect.y() + 3, 287, 24);
        painter->save();
        painter->translate(textRect.topLeft());
        painter->setClipRect(textRect.translated(-textRect.topLeft()));
        document.documentLayout()->draw(painter, paintContext);
        painter->restore();

    } else {
        auto playMusicTypePtr = playMusicTypePtrList[index.row()];
        /***********************绘制图片***************************/
        QPixmap image;
        image.loadFromData(playMusicTypePtr->icon);
        if (image.isNull()) {
            image = QPixmap(":/common/image/cover_max.svg");
        }
        painter->save();
        QRect imageRect(32, index.row() * 34 + 5, 24, 24);
        if (playlistPtr->id() == ArtistCandListID) {
            QPainterPath clipPath;
            clipPath.addEllipse(imageRect.adjusted(0, 0, 0, 0));
            painter->setClipPath(clipPath);
        } else {
            QPainterPath clipPath;
            clipPath.addRoundedRect(imageRect, 4, 4);
            painter->setClipPath(clipPath);
        }
        painter->drawPixmap(imageRect, image);
        painter->restore();

        QString mtext = playMusicTypePtr->name;
        QFontMetricsF fontWidth(textFont);
        mtext = fontWidth.elidedText(mtext, Qt::ElideMiddle, 251);
        QStyleOptionViewItem viewOption(option);
        initStyleOption(&viewOption, index);
        if (option.state.testFlag(QStyle::State_HasFocus))
            viewOption.state = viewOption.state ^ QStyle::State_HasFocus;
        QStyle *pStyle = viewOption.widget ? viewOption.widget->style() : QApplication::style();
        viewOption.text = "";
        pStyle->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);

        QTextDocument document;
        document.setPlainText(mtext);
        bool found = false;
        QTextCursor highlight_cursor(&document);
        QTextCursor cursor(&document);

        cursor.beginEditBlock();
        QTextCharFormat color_format(highlight_cursor.charFormat());

        color_format.setForeground(lightColor);

        /*-------------LineUnderCursor-------------*/
        QTextCursor testcursor(&document);
        testcursor.select(QTextCursor::LineUnderCursor);
        QTextCharFormat fmt;
        fmt.setForeground(textColor);
        testcursor.mergeCharFormat(fmt);
        testcursor.clearSelection();
        testcursor.movePosition(QTextCursor::EndOfLine);

        while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
            highlight_cursor = document.find(searchText, highlight_cursor);
            if (!highlight_cursor.isNull()) {
                if (!found)
                    found = true;
                highlight_cursor.mergeCharFormat(color_format);
            }
        }
        cursor.endEditBlock();

        QAbstractTextDocumentLayout::PaintContext paintContext;
        QRect textRect(61, option.rect.y() + 2, 251, 24);
        painter->save();
        painter->translate(textRect.topLeft());
        painter->setClipRect(textRect.translated(-textRect.topLeft()));
        document.documentLayout()->draw(painter, paintContext);
        painter->restore();
    }
}

QSize MusicSearchListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(350, 34);
}

QWidget *MusicSearchListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void MusicSearchListDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}

void MusicSearchListDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

//QString MusicSearchListDelegate::getElidedText(QFont font, QString str, int MaxWidth)
//{
//    if (str.isEmpty()) {
//        return "";
//    }
//    QFontMetrics fontWidth(font);
//    int width = fontWidth.width(str);
//    if (width >= MaxWidth) {
//        //右部显示省略号
//        str = fontWidth.elidedText(str, Qt::ElideRight, MaxWidth);
//    }
//    //返回处理后的字符串
//    return str;
//}

