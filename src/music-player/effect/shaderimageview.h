/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SHADERIMAGEVIEW_H
#define SHADERIMAGEVIEW_H
#include <QQuickPaintedItem>
#include <QImage>
#include <QVariant>


class ShaderImageView: public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QVariant presenter READ presenter WRITE setPresenter NOTIFY presenterChanged)

public:
    ShaderImageView(QQuickItem *parent = 0);
    ~ShaderImageView();

    void paint(QPainter *painter);
        //the paint method is already implemented in QQuickPaintedItem
        //you just override it
    QString source() const;
    void setSource(const QString &source);
    QVariant presenter() const;
    void setPresenter(const QVariant &presenter);
private:
    QString m_source;
    QVariant m_pPresenter;
signals:
    void sourceChanged();
    void presenterChanged();

};

#endif // SHADERIMAGEVIEW_H
