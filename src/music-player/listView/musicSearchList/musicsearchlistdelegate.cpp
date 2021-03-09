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

#include "musicsearchlistdelegate.h"
#include "musicsearchlistview.h"
#include "util/pinyinsearch.h"

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

MusicSearchListDelegate::MusicSearchListDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
    , m_shadowImg(DHiDPIHelper::loadNxPixmap(":/mpimage/light/shadow.svg"))
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
    // 设置基础颜色
    QColor backColor("#FFFFFF");
    backColor.setAlphaF(0.2);

    //获取当前行信息
    const MusicSearchListview *listview = qobject_cast<const MusicSearchListview *>(option.widget);
    QString searchText = listview->getSearchText();

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
        painter->drawRoundedRect(selecteColorRect, 0, 0);
        painter->restore();
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);

    // 设置字体
    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);

    if (listview->getSearchType() == SearchType::SearchMusic) {
        // 绘制歌曲
        MediaMeta metaPtr = index.data(Qt::UserRole + SearchType::SearchMusic).value<MediaMeta>();

        QString mtext;
        if (metaPtr.singer.size() == 0) {
            mtext = metaPtr.title;
        } else {
            mtext = metaPtr.title + " - " + metaPtr.singer;
        }
        QFontMetricsF fontWidth(fontT6);
        mtext = fontWidth.elidedText(mtext, Qt::ElideMiddle, 280);

        QStyleOptionViewItem viewOption(option);
        initStyleOption(&viewOption, index);
        if (option.state.testFlag(QStyle::State_HasFocus))
            viewOption.state = viewOption.state ^ QStyle::State_HasFocus;
        QStyle *pStyle = viewOption.widget ? viewOption.widget->style() : QApplication::style();
        viewOption.text = "";
        pStyle->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);

        QTextDocument document;
        // 设置文字边距，保证绘制文字居中
        document.setDocumentMargin(0);
        document.setPlainText(mtext);
        bool found = false;
        QTextCursor highlight_cursor(&document);
        QTextCursor cursor(&document);

        cursor.beginEditBlock();
        QTextCharFormat color_format(highlight_cursor.charFormat());
        color_format.setForeground(lightColor);

        // 搜索字体高亮
        QTextCursor testcursor(&document);
        testcursor.select(QTextCursor::LineUnderCursor);
        QTextCharFormat fmt;
        fmt.setForeground(textColor);
        fmt.setFont(fontT6);
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
        // 修复字体大小改变，字体显示不全问题，动态计算边距，保证调整字体大小时绘制居中
        int margin = static_cast<int>(((option.rect.height() - fontWidth.height()) / 2));
        QRect textRect(32, option.rect.y() + margin, 287, option.rect.height());
        painter->save();
        painter->translate(textRect.topLeft());
        painter->setClipRect(textRect.translated(-textRect.topLeft()));
        document.documentLayout()->draw(painter, paintContext);
        painter->restore();

    } else {
        // 绘制歌手专辑
        QPixmap image;
        QString mtext;
        image = index.data(Qt::UserRole + SearchType::SearchIcon).value<QPixmap>();
        painter->save();
        QRect imageRect(32, index.row() * 34 + 5, 24, 24);
        if (listview->getSearchType() == SearchType::SearchSinger) {
            QPainterPath clipPath;
            clipPath.addEllipse(imageRect.adjusted(0, 0, 0, 0));
            painter->setClipPath(clipPath);
            mtext = index.data(Qt::UserRole + SearchType::SearchSinger).value<SingerInfo>().singerName;
        } else {
            QPainterPath clipPath;
            clipPath.addRoundedRect(imageRect, 4, 4);
            painter->setClipPath(clipPath);
            mtext = index.data(Qt::UserRole + SearchType::SearchAlbum).value<AlbumInfo>().albumName;
        }
        painter->drawPixmap(imageRect, image);
        painter->restore();

        QFontMetricsF fontWidth(fontT6);
        mtext = fontWidth.elidedText(mtext, Qt::ElideMiddle, 251);
        QStyleOptionViewItem viewOption(option);
        initStyleOption(&viewOption, index);
        if (option.state.testFlag(QStyle::State_HasFocus))
            viewOption.state = viewOption.state ^ QStyle::State_HasFocus;
        QStyle *pStyle = viewOption.widget ? viewOption.widget->style() : QApplication::style();
        viewOption.text = "";
        pStyle->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);

        QTextDocument document;
        // 设置文字边距，保证绘制文字居中
        document.setDocumentMargin(0);
        document.setPlainText(mtext);
        bool found = false;
        QTextCursor highlight_cursor(&document);
        QTextCursor cursor(&document);

        cursor.beginEditBlock();
        QTextCharFormat color_format(highlight_cursor.charFormat());

        color_format.setForeground(lightColor);

        // 搜索字体高亮
        QTextCursor testcursor(&document);
        testcursor.select(QTextCursor::LineUnderCursor);
        QTextCharFormat fmt;
        fmt.setForeground(textColor);
        fmt.setFont(fontT6);
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
        // 动态计算边距，保证调整字体大小时绘制居中
        int margin = static_cast<int>(((option.rect.height() - fontWidth.height()) / 2));
        QRect textRect(61, option.rect.y() + margin, 251, option.rect.height());
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

//QWidget *MusicSearchListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    return QStyledItemDelegate::createEditor(parent, option, index);
//}

//void MusicSearchListDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
//{
//    QStyledItemDelegate::setEditorData(editor, index);
//}

//void MusicSearchListDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
//{
//    QStyledItemDelegate::setModelData(editor, model, index);
//}

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

