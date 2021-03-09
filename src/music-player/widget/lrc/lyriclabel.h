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

#ifndef LYRICLABEL_H
#define LYRICLABEL_H

#include <DWidget>
#include "musiclyric.h"
DWIDGET_USE_NAMESPACE

class AbstractWheelWidget : public DWidget
{
    Q_OBJECT

public:
    AbstractWheelWidget(bool touch, QWidget *parent = nullptr);
    virtual ~AbstractWheelWidget();

    int currentIndex() const;
    void setCurrentIndex(int index);

    bool event(QEvent *);
    void paintEvent(QPaintEvent *e);
    virtual void paintItem(QPainter *painter, int index, const QRect &rect) = 0;

    virtual int itemHeight() const = 0;
    virtual int itemCount() const = 0;

public slots:
    void scrollTo(int index);

signals:
    void stopped(int index);
    void changeTo(int index);

protected:
    int m_currentItem;
    int m_itemOffset;
    qreal m_lastY;
    bool isScrolled;
    bool dosignal;
    bool     m_FadeFlag = false;
    int      m_themetype = 1;
    int currentWheelNum = 0;
};

//class MusicLyric;
class QFont;

class LyricLabel : public AbstractWheelWidget
{
    Q_OBJECT

public:
    LyricLabel(bool touch, QWidget *parent = nullptr);
    void getFromFile(QString dir);
signals:
    void changeTo(qint64 pos);
    void rightClicked();
    void mouseEnter();

public slots:
    void postionChanged(qint64 pos);
    void setThemeType(int type);
protected slots:
    void changeToEvent(int index);
    //void changeFont();

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;//右击事件
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    virtual void paintItem(QPainter *painter, int index, const QRect &rect) override;
    virtual int itemHeight() const override;
    virtual int itemCount() const override;

protected:
    MusicLyric lyric;
    QFont lyricFont;
    QColor lyricNormal;
    QColor lyricHighlight;
};

#endif // LYRICLABEL_H
