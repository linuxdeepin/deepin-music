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

#include <QWidget>

class Cover : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor NOTIFY shadowColorChanged)
    Q_PROPERTY(QString backgroundUrl READ backgroundUrl WRITE setBackgroundUrl NOTIFY backgroundUrlChanged)

public:
    explicit Cover(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

    QColor borderColor() const
    {
        return m_borderColor;
    }

    QColor shadowColor() const
    {
        return m_shadowColor;
    }

    QString backgroundUrl() const
    {
        return m_backgroundUrl;
    }

signals:
    void borderColorChanged(QColor borderColor);
    void shadowColorChanged(QColor shadowColor);
    void backgroundUrlChanged(QString backgroundUrl);

public slots:
    void setBorderColor(QColor borderColor)
    {
        if (m_borderColor == borderColor) {
            return;
        }

        m_borderColor = borderColor;
        emit borderColorChanged(borderColor);
    }
    void setShadowColor(QColor shadowColor)
    {
        if (m_shadowColor == shadowColor) {
            return;
        }

        m_shadowColor = shadowColor;
        emit shadowColorChanged(shadowColor);
    }
    void setBackgroundUrl(QString backgroundUrl)
    {
        if (m_backgroundUrl == backgroundUrl) {
            return;
        }

        m_backgroundUrl = backgroundUrl;
        setBackgroundImage(QPixmap(backgroundUrl));
        emit backgroundUrlChanged(backgroundUrl);
    }

private:
    void setBackgroundImage(const QPixmap &bk);

    QColor  m_borderColor;
    QColor  m_shadowColor;
    QString m_backgroundUrl;
    QPixmap m_Background;
};

#endif // COVER_H
