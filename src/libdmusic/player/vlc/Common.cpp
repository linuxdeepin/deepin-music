// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Common.h"
#include "util/log.h"

QStringList VlcCommon::args()
{
    qCDebug(dmMusic) << "VLC args:" << qgetenv("VLC_ARGS");
    QStringList args_list;

    QString args = qgetenv("VLC_ARGS");
    if (!args.isEmpty()) {
        qCDebug(dmMusic) << "VLC args:" << args;
        args_list << args.split(u' ', Qt::SkipEmptyParts);
    } else {
        qCDebug(dmMusic) << "VLC args not found, using default args";
        args_list << "--intf=dummy"
                  << "--no-media-library"
                  << "--no-stats"
                  << "--no-osd"
                  << "--no-loop"
                  << "--no-video-title-show"
                  << "--drop-late-frames";
    }

    qCDebug(dmMusic) << "VLC return args:" << args_list;
    return args_list;
}

//bool VlcCommon::setPluginPath(const QString &path)
//{
//    if (qgetenv("VLC_PLUGIN_PATH").isEmpty()) {
//        return qputenv("VLC_PLUGIN_PATH", path.toLocal8Bit());
//    }

//    return false;
//}
