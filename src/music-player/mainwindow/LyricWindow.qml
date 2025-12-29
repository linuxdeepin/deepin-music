// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.2
import "../lyric/"
//import "."

LyricPage {
    id: lyricPage
    width: rootWindow.width
    height: rootWindow.height
    visible: false

    signal animationFinished(var lyircShow)
    signal animationStart(var show)

    ParallelAnimation {
        id: lyricRaiseAnimation
        YAnimator {
            target: lyricPage
            from: rootWindow.height
            to: -50
            duration: 500
            easing.type: Easing.OutCubic
        }

        OpacityAnimator {
            target: lyricPage
            from: 0
            to: 1
            duration: 500
            easing.type: Easing.InQuart
        }
    }

    ParallelAnimation {
        id: lyricHideAnimation
        YAnimator {
            target: lyricPage
            from: -50
            to: rootWindow.height
            duration: 350
            easing.type: Easing.OutCubic
        }

        // OpacityAnimator {
        //     target: lyricPage
        //     from: 1
        //     to: 0
        //     duration: 350
        //     easing.type: Easing.OutQuint
        // }
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
        if (lyricPage.visible) {
            lyricHideAnimation.start()
        } else {
            lyricPage.y = rootWindow.height
            lyricRaiseAnimation.start()
            lyricPage.visible = true
        }
    }

    Connections {
        target: lyricHideAnimation
        onStopped: {
            lyricPage.visible = false
        }
        onStarted: {
            animationStart(false)
            // 动画开始时就更新状态，让 contentWindow 提前显示，避免空白
            animationFinished(true)
        }
    }
    Connections {
        target:lyricRaiseAnimation
        onFinished: {
            animationFinished(false)
        }
        onStarted: {
            animationStart(true)
        }
    }

    Connections {
        target: rootWindow
        onClickForLyricUp: {
            if (lyricPage.visible) {
                lyricHideAnimation.start()
            } else {
                lyricRaiseAnimation.start()
                lyricPage.visible = true
            }
        }
    }

}
