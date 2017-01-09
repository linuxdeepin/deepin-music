/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QScopedPointer>
#include <QWidget>

class ThinWindowPrivate;
class ThinWindow : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int radius READ radius WRITE setRadius)
    Q_PROPERTY(QBrush background READ background WRITE setBackground)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
    Q_PROPERTY(bool borderInside READ borderInside WRITE setBorderInside NOTIFY borderInsideChanged)
    Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor NOTIFY shadowColorChanged)
    Q_PROPERTY(QPoint shadowOffset READ shadowOffset WRITE setShadowOffset NOTIFY shadowOffsetChanged)
    Q_PROPERTY(int shadowWidth READ shadowWidth WRITE setShadowWidth NOTIFY shadowWidthChanged)

public:
    explicit ThinWindow(QWidget *parent = 0);
    ~ThinWindow();

    void setContentLayout(QLayout *);

    void resize(QSize sz);
    QSize size() const;
    QRect rect() const;

    int radius() const;
    QColor borderColor() const;
    QBrush background() const;
    QColor shadowColor() const;
    QPoint shadowOffset() const;
    bool borderInside() const;
    int shadowWidth() const;

public slots:
    void showMinimized();
    void showMaximized();
    void showFullScreen();
    void showNormal();

    void moveWindow(Qt::MouseButton botton);
    void toggleMaximizedWindow();

    void setBackgroundImage(QPixmap);
    void setBackground(QBrush background);
    void setRadius(int radius);
    void setBorderColor(QColor borderColor);
    void setShadowColor(QColor shadowColor);
    void setShadowOffset(QPoint shadowOffset);
    void setBorderInside(bool borderInside);
    void setShadowWidth(int shadowWidth);

signals:
    void shadowColorChanged(QColor shadowColor);
    void shadowOffsetChanged(QPoint shadowOffset);
    void borderInsideChanged(bool borderInside);
    void shadowWidthChanged(int shadowWidth);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    virtual void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

private:
    QScopedPointer<ThinWindowPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ThinWindow)
    int m_shadowWidth;
};

class FilterMouseMove : public QObject
{
    Q_OBJECT

public:
    explicit FilterMouseMove(QObject *object = nullptr);
    ~FilterMouseMove();

    bool eventFilter(QObject *obj, QEvent *event);

    QWidget *m_rootWidget = nullptr;
};

