// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VLCDYNAMICINSTANCE_H
#define VLCDYNAMICINSTANCE_H

#include <QObject>
#include <QMap>
#include <QLibrary>
#include <QMutex>

class VlcDynamicInstance : public QObject
{
    Q_OBJECT
public:
    static VlcDynamicInstance *VlcFunctionInstance();

    QFunctionPointer resolveSymbol(const char *symbol, bool bffmpeg = false);

    QFunctionPointer resolveSdlSymbol(const char *symbol);

    bool loadSdlLibrary();

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

private:
    QLibrary libcore;
    QLibrary libdvlc;
    QLibrary libavcode;
    QLibrary libdformate;
    QLibrary libsdl2;

    QMap<QString, QFunctionPointer> m_funMap;
    QMutex m_funMapMutex;  // 添加互斥锁
};

#endif // VLCDYNAMICINSTANCE_H
