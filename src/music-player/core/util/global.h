// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    // 检测是否boardVendor
    static bool checkBoardVendorType();
    // 开启boardVendor
    static void setBoardVendorType(bool type);
    // boardVendor
    static bool boardVendorType();
};
