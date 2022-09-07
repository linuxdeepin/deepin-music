// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VLCDYNAMICINSTANCE_H
#define VLCDYNAMICINSTANCE_H

#include <QObject>
#include <QMap>
#include <QLibrary>

class VlcDynamicInstance : public QObject
{
    Q_OBJECT
public:
    static VlcDynamicInstance *VlcFunctionInstance();
    QFunctionPointer resolveSymbol(const char *symbol, bool bffmpeg = false);
signals:

public slots:

private:
    explicit VlcDynamicInstance(QObject *parent = nullptr);
    ~VlcDynamicInstance();
    bool loadVlcLibrary();
    /**
     * @brief libPath get absolutely library path
     * @param strlib library name
     * @return
     */
    QString libPath(const QString &strlib);
    QLibrary libcore;
    QLibrary libdvlc;
    QLibrary libavcode;
    QLibrary libdformate;

    QMap<QString, QFunctionPointer> m_funMap;
};

#endif // VLCDYNAMICINSTANCE_H
