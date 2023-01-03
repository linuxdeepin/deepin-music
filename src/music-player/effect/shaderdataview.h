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
#ifndef SHADERDATAVIEW_H
#define SHADERDATAVIEW_H
#include <QQuickPaintedItem>
#include <QImage>
#include <QVariant>

class Presenter;
class ShaderDataView: public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QVariant presenter READ presenter WRITE setPresenter NOTIFY presenterChanged)

public:
    ShaderDataView(QQuickItem *parent = 0);
    ~ShaderDataView();

    void paint(QPainter *painter);
        //the paint method is already implemented in QQuickPaintedItem
        //you just override it
    QVector<int> shaderData() const;
    void setShaderData(const QVector<int> &shaderData);
    QVariant presenter() const;
    void setPresenter(const QVariant &presenter);
private:
    QVector<int> m_shaderData;
    QImage m_img;
    QVariant m_pPresenter;
    Presenter *m_workPre;
public slots:
    void slotAudioData(QVector<int> audioData);
signals:
    void shaderDataChanged();
    void presenterChanged();
};

#endif // SHADERDATAVIEW_H
