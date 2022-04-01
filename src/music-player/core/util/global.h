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

#pragma once

#include <QString>

class Global
{
public:
    // 获取config路径
    static QString configPath();
    // 获取缓存路径
    static QString cacheDir();
    // 设置应用名称
    static void setAppName(const QString &name);
    // 获取应用名称
    static QString getAppName();
    // 检测是否开启Wayland
    static bool checkWaylandMode();
    // 设置Wayland
    static void setWaylandMode(bool mode);
    // 是否开启Wayland
    static bool isWaylandMode();
    // 初始化播放引擎类型
    static void initPlaybackEngineType();
    // 播放引擎类型1为vlc，0为QMediaPlayer
    static int playbackEngineType();
};
