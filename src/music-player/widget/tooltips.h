// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLTIPS_H
#define TOOLTIPS_H
#include <QFrame>
#include <QGuiApplication>
#include <DPalette>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <QApplication>
#include <QDesktopWidget>
#include <DLabel>


DWIDGET_USE_NAMESPACE

class ToolTips : public QFrame
{
    Q_OBJECT
public:
    explicit ToolTips(const QString &text,
                      QWidget *parent = nullptr);
    ~ToolTips() override;

//    void pop(QPoint center);

public slots:
    void setText(const QString &text);
private slots:
    void setThemeType(int type);

protected:
    virtual void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) override;
private:

    QPen            m_pen;
    QBrush          m_brush;
    int             radius              = 8;
    int             shadowWidth         = 20;
    QMargins        shadowMargins       = QMargins(20, 20, 20, 20);
    QColor          borderColor         = QColor(0, 0, 0, 51/*0.2 * 255*/);

    DLabel          *textLable          = nullptr;
    QFrame          *m_interFrame       = nullptr;
    QString m_strText;
};


#endif // TOOLTIPS_H
