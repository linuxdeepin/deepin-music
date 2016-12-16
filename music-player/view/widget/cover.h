/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef COVER_H
#define COVER_H

#include <QFrame>

class Cover : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor NOTIFY shadowColorChanged)

public:
    explicit Cover(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

    QColor borderColor() const
    {
        return m_borderColor;
    }

    int radius() const
    {
        return m_radius;
    }

    QColor shadowColor() const
    {
        return m_shadowColor;
    }

signals:
    void borderColorChanged(QColor borderColor);
    void radiusChanged(int radius);
    void shadowColorChanged(QColor shadowColor);

public slots:
    void setBorderColor(QColor borderColor)
    {
        if (m_borderColor == borderColor) {
            return;
        }

        m_borderColor = borderColor;
        emit borderColorChanged(borderColor);
    }

    void setBackgroundImage(const QPixmap &backgroundPixmap);

    void setRadius(int radius)
    {
        if (m_radius == radius) {
            return;
        }

        m_radius = radius;
        emit radiusChanged(radius);
    }
    void setShadowColor(QColor shadowColor)
    {
        if (m_shadowColor == shadowColor)
            return;

        m_shadowColor = shadowColor;
        emit shadowColorChanged(shadowColor);
    }

private:

    int     m_radius;
    QColor  m_borderColor;
    QColor  m_shadowColor;
    QString m_backgroundUrl;
    QPixmap m_Background;
};

#endif // COVER_H
