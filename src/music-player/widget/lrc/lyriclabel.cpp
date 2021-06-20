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

#include "lyriclabel.h"

#include <QScroller>
#include <QEvent>
#include <QScrollPrepareEvent>
#include <QPainter>
#include <QApplication>
#include <QFontDialog>
#include <QColorDialog>
#include <DPalette>
#include <DGuiApplicationHelper>

#include "../../core/util/musiclyric.h"
#include "../../core/musicsettings.h"
#include "musicsettings.h"

#include "ac-desktop-define.h"

#define WHEEL_SCROLL_OFFSET 50000.0

#include <QDebug>

DGUI_USE_NAMESPACE

LyricLabel::LyricLabel(bool touch, QWidget *parent)
    : AbstractWheelWidget(touch, parent)
{
    AC_SET_OBJECT_NAME(this, AC_lyricview);
    AC_SET_ACCESSIBLE_NAME(this, AC_lyricview);

//    lyric = new MusicLyric();
//    lyricFont = new QFont();
    lyricFont.setFamily("SourceHanSansSC");
    lyricFont.setWeight(QFont::Normal);

// 解决字体不会根据系统字体大小改变问题
//    lyricFont.setPixelSize(14);
    lyricNormal = QColor("#526A7F");
    lyricHighlight = QColor("#000000");
    connect(this, &LyricLabel::changeTo, this, &LyricLabel::changeToEvent);

    m_FadeFlag = MusicSettings::value("base.play.fade_in_out").toBool();


    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &LyricLabel::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());

    /* MyMenu *menu = new MyMenu(this);
     QAction *selectLyric = new QAction("关联本地歌词", menu);
     QAction *fontSelect = new QAction("字体设置", menu);
     connect(fontSelect, SIGNAL(triggered(bool)), this, SLOT(changeFont()));
    // QAction *colorNormal = new QAction("普通颜色", menu);
    QAction *colorNormal = new QAction("pp", menu);
     connect(colorNormal, SIGNAL(triggered(bool)), this, SLOT(changeNormalColor()));
     QAction *colorHighLight = new QAction("高亮颜色", menu);
     connect(colorHighLight, SIGNAL(triggered(bool)), this, SLOT(changeHightLightColor()));
     menu->addAction(selectLyric);
     menu->addSeparator();
     menu->addAction(fontSelect);
     menu->addAction(colorNormal);
     menu->addAction(colorHighLight);
     connect(this, SIGNAL(rightClicked()), menu, SLOT(menuVisiable()));*/
    //setMinimumWidth(600);
}

void LyricLabel::getFromFile(QString dir)
{
    lyric.getFromFile(dir);
    this->update();
}

void LyricLabel::paintItem(QPainter *painter, int index, const QRect &rect)
{
    if (index == this->m_currentItem) {
        painter->setPen(lyricHighlight);
        QFont font(lyricFont);
        font.setPixelSize(font.pixelSize() + 1);
        painter->setFont(font);
        QPoint leftpos = rect.bottomLeft();
        leftpos.setY(static_cast<int>(leftpos.y() + rect.height() / 2.0 + 5));
        QPoint rightpos = rect.bottomRight();
        rightpos.setY(leftpos.y());
        rightpos.setX(rightpos.x() - 3);
        leftpos.setY(leftpos.y() - 3);
        rightpos.setY(rightpos.y() - 3);
        //leftpos.setY(leftpos.y() - rect.height() / 2);
        //rightpos.setY(rightpos.y() - rect.height() / 2);
        painter->save();
        QPointF triangle1[3] = {QPointF(leftpos.x(), leftpos.y() * 1.0 + 4.5), QPointF(leftpos.x(), leftpos.y() * 1.0 - 4.5), QPointF(leftpos.x() + 9, leftpos.y())}; //1
        QPointF triangle2[3] = {QPointF(rightpos.x(), rightpos.y() * 1.0 + 4.5), QPointF(rightpos.x(), rightpos.y() * 1.0 - 4.5), QPointF(rightpos.x() - 9, rightpos.y())}; //1

        painter->setRenderHint(QPainter::Antialiasing, true);

        QColor normalcolor = lyricHighlight;
        normalcolor.setAlphaF(0.5);
        QPen pen(normalcolor, 1, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);

        painter->setBrush(normalcolor);

        painter->setPen(pen);

        painter->drawPolygon(triangle1, 3); //2
        painter->drawPolygon(triangle2, 3); //2

        painter->drawLine(leftpos.x() + 23, leftpos.y(), rightpos.x() - 23, rightpos.y());
        painter->restore();
    } else {
        QPen pen = painter->pen();
        QColor color = pen.color();
        color.setRed(lyricNormal.red());
        color.setGreen(lyricNormal.green());
        color.setBlue(lyricNormal.blue());
        painter->setPen(color);
        painter->setFont(lyricFont);
    }
    QFontMetrics fm(lyricFont);
    QString lricstr = lyric.getLineAt(index);
    QPoint tleftpos = rect.topLeft();
    if (fm.width(lricstr)  > rect.width() - 100) {

        QString str1, str2;
        for (int i = 0; i < lricstr.count(); i++) {
            str1.append(lricstr.at(i));
            if (fm.width(str1) > rect.width() - 100) {
                str1.chop(1);
                break;
            }
        }
        str2 = lricstr.remove(str1);

        painter->drawText(QRect(tleftpos.x(), tleftpos.y(), rect.width(), rect.height() / 2), Qt::AlignCenter, str1);
        painter->drawText(QRect(tleftpos.x(), tleftpos.y() + rect.height() / 2, rect.width(), rect.height() / 2), Qt::AlignCenter, str2);

    } else {
        painter->drawText(QRect(tleftpos.x(), tleftpos.y(), rect.width(), rect.height() / 2), Qt::AlignCenter, lyric.getLineAt(index));
    }

}

int LyricLabel::itemHeight() const
{
    QFontMetrics fm(lyricFont);
    //qDebug() << "itemheight" << fm.height()*2.8;
    //return fm.height() * 1.4;
    return static_cast<int>(fm.height() * 2.8);
    //return 45;
}

int LyricLabel::itemCount() const
{
    return lyric.getCount();
}

void LyricLabel::postionChanged(qint64 pos)
{
    if (this->isScrolled) return;
    pos = pos + 500; //歌词滚动需要500ms
    int index = lyric.getIndex(pos) - 1;
    if (index != m_currentItem)
        this->scrollTo(index);
}

void LyricLabel::setThemeType(int type)
{
    m_themetype = type;
    if (type == 1) {
        QColor normalcolor = "#000000";
        normalcolor.setAlphaF(0.5);
        lyricNormal =  normalcolor;
        lyricHighlight =  QColor("#000000");
    } else {
        lyricNormal =  QColor("#C0C6D4");
        lyricHighlight =  QColor("#FFFFFF");
    }
    update();
}

void LyricLabel::changeToEvent(int index)
{
    emit changeTo(lyric.getPosition(index));
}

//void LyricLabel::changeFont()
//{
//    bool flag;
//    *lyricFont = QFontDialog::getFont(&flag, *lyricFont, this);
//    if (flag) {
//        // the user clicked OK and font is set to the font the user selected
//    } else {
//        // the user canceled the dialog; font is set to the initial value
//        lyricFont = new QFont("宋体", 12, QFont::Bold);
//    }
//}

void LyricLabel::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event)
    emit rightClicked();
}

void LyricLabel::enterEvent(QEvent *e)
{
    Q_UNUSED(e)
    emit mouseEnter();
}

AbstractWheelWidget::AbstractWheelWidget(bool touch, QWidget *parent)
    : QWidget(parent), m_currentItem(0), m_itemOffset(0)
{
// ![0]
    QScroller::grabGesture(this, touch ? QScroller::TouchGesture : QScroller::LeftMouseButtonGesture);
// ![0]
    this->isScrolled = false;
    this->dosignal = true;
}

AbstractWheelWidget::~AbstractWheelWidget()
{ }

int AbstractWheelWidget::currentIndex() const
{
    return m_currentItem;
}

void AbstractWheelWidget::setCurrentIndex(int index)
{
    if (index >= 0 && index < itemCount()) {
        m_currentItem = index;
        m_itemOffset = 0;
        update();
    }
}

bool AbstractWheelWidget::event(QEvent *e)
{
    switch (e->type()) {
// ![1]
    case QEvent::ScrollPrepare: {
        // We set the snap positions as late as possible so that we are sure
        // we get the correct itemHeight
        QScroller *scroller = QScroller::scroller(this);
        scroller->setSnapPositionsY(WHEEL_SCROLL_OFFSET, itemHeight());

        QScrollPrepareEvent *se = static_cast<QScrollPrepareEvent *>(e);
        se->setViewportSize(QSizeF(size()));
        // we claim a huge scrolling area and a huge content position and
        // hope that the user doesn't notice that the scroll area is restricted
        se->setContentPosRange(QRectF(0.0, 0.0, 0.0, WHEEL_SCROLL_OFFSET * 2));
        se->setContentPos(QPointF(0.0, WHEEL_SCROLL_OFFSET + m_currentItem * itemHeight() + m_itemOffset));
        se->accept();
        return true;
    }
// ![1]
// ![2]
    case QEvent::Scroll: {
        QScrollEvent *se = static_cast<QScrollEvent *>(e);
        if (dosignal) {

            if (se->scrollState() == QScrollEvent::ScrollStarted) {
                qDebug() << "start scroll lyric" << endl;
                this->isScrolled = true;
            }
        }
        if (se->scrollState() == QScrollEvent::ScrollFinished) {

            if (dosignal) {
                qDebug() << "滚动到第" << m_currentItem << endl;
                emit changeTo(this->m_currentItem);
            }
            this->isScrolled = false;
            dosignal = true;
        }

        qreal y = se->contentPos().y();
        int iy = static_cast<int>(y - WHEEL_SCROLL_OFFSET);
        int ih = itemHeight();

// ![2]
// ![3]
        // -- calculate the current item position and offset and redraw the widget
        int ic = itemCount();
        if (ic > 0) {
            m_currentItem = iy / ih;
            //m_currentItem = iy / ih % ic;
            m_itemOffset = iy % ih;

            // take care when scrolling backwards. Modulo returns negative numbers
            /*
            if (m_itemOffset < 0) {
                m_itemOffset += ih;
                m_currentItem--;
            }
            */
            //if (m_currentItem < 0)
            //m_currentItem += ic;
            //m_currentItem = 0;
            if (m_currentItem >= ic) m_currentItem = ic - 1;
        }
        // -- repaint
        update();

        se->accept();
        return true;
// ![3]
    }
    case QEvent::Wheel: {
        QWheelEvent *we = static_cast<QWheelEvent *>(e);

        int delta = we->delta();
        //过滤觸控板双指和鼠标滚轮
        if (abs(delta) < 120) { //觸控板
            currentWheelNum += delta;

            if (abs(currentWheelNum) < 120) {
                we->accept();
                return true;
            }
            if (currentWheelNum > 0) {
                this->m_currentItem -= 1;
            } else {
                this->m_currentItem += 1;
            }
            currentWheelNum = 0;
        } else {// 鼠标滚轮
            if (delta > 0) {
                this->m_currentItem -= 1;
            } else {
                this->m_currentItem += 1;
            }
        }

        if (this->m_currentItem < 0)
            this->m_currentItem = 0;

        int ic = itemCount();
        if (m_currentItem >= ic)
            m_currentItem = ic - 1;

        emit changeTo(this->m_currentItem);
        update();
        we->accept();
        return true;
    }
    case QEvent::MouseButtonPress:
        return true;
    default:
        return QWidget::event(e);
    }
//    return true;
}

void AbstractWheelWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // -- first calculate size and position.
    int w = width();
    int h = height();

    QPainter painter(this);
    DPalette palette = this->palette();
    DPalette::ColorGroup colorGroup = isEnabled() ? DPalette::Active : DPalette::Disabled;

    // linear gradient brush
    QLinearGradient grad(0.5, 0, 0.5, 1.0);
    grad.setColorAt(0, palette.color(colorGroup, DPalette::ButtonText));
    grad.setColorAt(0.2, palette.color(colorGroup, DPalette::Button));
    grad.setColorAt(0.8, palette.color(colorGroup, DPalette::Button));
    grad.setColorAt(1.0, palette.color(colorGroup, DPalette::ButtonText));
    grad.setCoordinateMode(QGradient::ObjectBoundingMode);
    QBrush gBrush(grad);
    /*
        // paint a border and background
        painter.setPen(palette.color(colorGroup, QPalette::ButtonText));
        painter.setBrush(gBrush);
        // painter.setBrushOrigin( QPointF( 0.0, 0.0 ) );
        //painter.drawRect( 0, 0, w-1, h-1 );

        // paint inner border
        painter.setPen(palette.color(colorGroup, QPalette::Button));
        painter.setBrush(Qt::NoBrush);
        //painter.drawRect( 1, 1, w-3, h-3 );
    */
    // paint the items
    painter.setClipRect(QRect(3, 3, w - 6, h - 6));
    painter.setPen(palette.color(colorGroup, QPalette::ButtonText));

    int iH = itemHeight();
    int iC = itemCount();
    if (iC > 0) {

        m_itemOffset = m_itemOffset % iH;

        for (int i = -h / 2 / iH; i <= h / 2 / iH + 1; i++) {

            int itemNum = m_currentItem + i;
            /*
            while (itemNum < 0)
                itemNum += iC;
            while (itemNum >= iC)
                itemNum -= iC;
            */
            if (itemNum >= 0 && itemNum < iC) {
                int len = h / 2 / iH;
                /*线性衰减的方法
                int t = len-abs(i);
                t = (t+8)*255/(len+8);
                */
                //抛物线衰减的方法
                int t = abs(i);
                t = 255 - t * t * 220 / len / len - 35; //220是255-y得到,y为边界透明度
                if (t < 0) t = 0;
                //qDebug() << "a值:" << t << endl;
                if (m_themetype == 1) {
                    if (m_FadeFlag) {
                        painter.setPen(QColor(255, 255, 255, t));
                    } else {
                        painter.setPen(QColor(255, 255, 255, 255));
                    }
                } else {
                    if (m_FadeFlag) {
                        painter.setPen(QColor(0, 0, 0, t));
                    } else {
                        painter.setPen(QColor("#FFFFFF"));
                    }
                }

                // QRect第二个参数-50用于item向上偏移
                paintItem(&painter, itemNum, QRect(6, h / 2 + i * iH - m_itemOffset - iH / 2 - 50, w - 6, iH));
            }
        }
    }
    /*
        // draw a transparent bar over the center
        QColor highlight = palette.color(colorGroup, QPalette::Highlight);
        highlight.setAlpha(150);

        QLinearGradient grad2(0.5, 0, 0.5, 1.0);
        grad2.setColorAt(0, highlight);
        grad2.setColorAt(1.0, highlight.lighter());
        grad2.setCoordinateMode( QGradient::ObjectBoundingMode );
        QBrush gBrush2( grad2 );

        QLinearGradient grad3(0.5, 0, 0.5, 1.0);
        grad3.setColorAt(0, highlight);
        grad3.setColorAt(1.0, highlight.darker());
        grad3.setCoordinateMode( QGradient::ObjectBoundingMode );
        QBrush gBrush3( grad3 );

        painter.fillRect( QRect( 0, h/2 - iH/2, w, iH/2 ), gBrush2 );
        painter.fillRect( QRect( 0, h/2,        w, iH/2 ), gBrush3 );
    */
}

/*!
    Rotates the wheel widget to a given index.
    You can also give an index greater than itemCount or less than zero in which
    case the wheel widget will scroll in the given direction and end up with
    (index % itemCount)
*/
void AbstractWheelWidget::scrollTo(int index)
{
    this->dosignal = false;
    setCurrentIndex(index);
//    QScroller *scroller = QScroller::scroller(this);
//    scroller->scrollTo(QPointF(0, WHEEL_SCROLL_OFFSET + index * itemHeight()), 500);
}
