// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shaderdataview.h"
#include <QPainter>
#include <qmath.h>
#include <QTimer>
#include <qrandom.h>
#include <presenter.h>


ShaderDataView::ShaderDataView(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    m_workPre = NULL;
}

ShaderDataView::~ShaderDataView()
{
}



QVector<int> ShaderDataView::shaderData() const
{
    return m_shaderData;
}

void ShaderDataView::setShaderData(const QVector<int> &data)
{
    if (data.isEmpty()) return;
    m_shaderData = data;
    int nWidth = data.count();
    m_img =  QImage(QSize(nWidth, 1), QImage::Format_ARGB32);
    unsigned char *line = m_img.scanLine(0);
    for (int i = 0; i < nWidth; i++) {
        unsigned int rgb = qRgba(data[i], 0, 0, 255);
        unsigned char *c = (unsigned char *)(&rgb);
        unsigned char a = *(c);
        unsigned char r = *(c + 1);
        unsigned char g = *(c + 2);
        unsigned char b = *(c + 3);
        *line = a;
        line++;
        *line = r;
        line++;
        *line = g;
        line++;
        *line = b;
        line++;
    }
    update();
}

QVariant ShaderDataView::presenter() const
{

}

void ShaderDataView::setPresenter(const QVariant &presenter)
{
    m_pPresenter = presenter;
    if (m_pPresenter.canConvert<Presenter *>()) { //判断防止空指针
        m_workPre = m_pPresenter.value<Presenter *>();
        connect(m_workPre, &Presenter::audioSpectrumData, this, &ShaderDataView::slotAudioData);
    }
    emit presenterChanged();
}

void ShaderDataView::slotAudioData(QVector<int> audioData)
{
    m_shaderData.clear();

    float nMax = 2048.0;
    //重采样数据
    foreach(int data, audioData) {
        if(data > nMax) data = nMax;
        m_shaderData.append(int((data/nMax*128)));
    }
    setShaderData(m_shaderData);
}



void ShaderDataView::paint(QPainter *painter)
{
    QRect rect(0, 0, static_cast<int>(width()), static_cast<int>(height()));
    painter->drawImage(rect, m_img, rect);
}
