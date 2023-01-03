// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk 1.0
import org.deepin.dtk.settings 1.0

Config {  // 创建配置项 用于关联到SettingDialog中
    id: config
    name: "org.deepin.music"

    property bool autoPlay: false
    property bool rememberProgress: true
    property bool fadeInOut: false
    property int closeAction: 0

    //shortcuts
    property string shortcutPlayPause: ""
    property string shortcutPrevious: ""
    property string shortcutNext: ""
    property string shortcutVolumeUp: ""
    property string shortcutVolumeDown: ""
    property string shortcutFavoriteSong: ""
}
