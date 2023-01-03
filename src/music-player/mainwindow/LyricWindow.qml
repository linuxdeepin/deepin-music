// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import "../lyric/"
//import "."

LyricPage {
    id: lyricPage
    width: rootWindow.width
    height: rootWindow.height
    visible: isLyricShow

    NumberAnimation on y {
        id: lyricRaiseAnimation
        running: false
        to: -50
        duration: 200
        easing.type: Easing.OutCubic
    }
    NumberAnimation on y {
        id: lyricHideAnimation
        running: false
        from: -50
        to: rootWindow.height
        duration: 200
        easing.type: Easing.OutCubic
    }

    /*function lrcUp() {
        if (isLyricShow) {
            lyricHideAnimation.start()
        } else {
            lyricPage.y = rootWindow.height
            lyricRaiseAnimation.start()
            isLyricShow = true
        }
    }*/

    function lyricWindowUp() {
        if (isLyricShow) {
            lyricHideAnimation.start()
        } else {
            lyricPage.y = rootWindow.height
            lyricRaiseAnimation.start()
            isLyricShow = true
        }
    }

    Connections {
        target: lyricHideAnimation
        onStopped: {
            isLyricShow = false
        }
    }
    Connections {
        target: rootWindow
        onClickForLyricUp: {
            if (isLyricShow) {
                lyricHideAnimation.start()
            } else {
                lyricPage.y = rootWindow.height
                lyricRaiseAnimation.start()
                isLyricShow = true
            }
        }
    }

}
