// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABLETLABEL_H
#define TABLETLABEL_H

#include <QLabel>
#include <QWidget>

class TabletLabel : public QLabel
{
    Q_OBJECT
public:
    TabletLabel(const QString &text, QWidget *parent = nullptr, int val = 0);
signals:
    void signalTabletSelectAll();
    void signalTabletDone();
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
public slots:
private:
    int m_valprop = 0;
};
#endif // TABLETLABEL_H
