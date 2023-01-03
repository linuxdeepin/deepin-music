// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
