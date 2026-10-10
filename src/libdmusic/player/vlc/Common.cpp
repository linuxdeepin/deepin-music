// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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
                  << "--no-media-library"     // 禁用媒体库
                  << "--no-stats"             // 禁用统计信息
                  << "--no-spu"               // 禁用子画面
                  << "--no-osd"               // 禁用屏幕显示（OSD）
                  << "--no-loop"              // 禁止全部循环
                  << "--no-video-title-show"  // 禁用在视频上显示媒体标题
                  << "--drop-late-frames"     // 丢弃延迟的帧
                  << "--no-video";            // 禁用视频输出
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
