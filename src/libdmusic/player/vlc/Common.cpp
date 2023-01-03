/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"

QStringList VlcCommon::args()
{
    QStringList args_list;

    QString args = qgetenv("VLC_ARGS");
    if (!args.isEmpty())
        args_list << args.split(" ", QString::SkipEmptyParts);
    else {
        args_list << "--intf=dummy"
                  << "--no-media-library"
                  << "--no-stats"
                  << "--no-osd"
                  << "--no-loop"
                  << "--no-video-title-show"
                  << "--drop-late-frames";
    }

    return args_list;
}

//bool VlcCommon::setPluginPath(const QString &path)
//{
//    if (qgetenv("VLC_PLUGIN_PATH").isEmpty()) {
//        return qputenv("VLC_PLUGIN_PATH", path.toLocal8Bit());
//    }

//    return false;
//}
