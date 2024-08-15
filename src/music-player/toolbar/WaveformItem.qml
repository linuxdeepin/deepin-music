// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import Qt5Compat.GraphicalEffects

Rectangle {
    property int sampleCount: Math.abs(parent.width / sampleRectWidth / 2)
    property int sampleRectWidth: 2
    property int waveformHeight: 30

    property color stripedStartColor
    property color stripedEndColor

    width: 360
    height: 30
    color: "#00000000"

    //灰色背景区域
    Rectangle {
        id: backgroundRect
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
        color: "#00000000"

        StripedRectangle {
            width: parent.width
            height: parent.height
            anchors.centerIn: parent
            itemWidth: sampleRectWidth
            itemCount: sampleCount
            chromatic: false
        }
    }

    //彩色活动区域
    Rectangle {
        id: activeRect
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
        color: "#00000000"
        visible: false

        StripedRectangle {
            width: parent.width
            height: parent.height
            anchors.centerIn: parent
            itemWidth: sampleRectWidth
            itemCount: sampleCount
            chromatic: true
            startColor: stripedStartColor
            endColor: stripedEndColor
        }
    }
    Item {
        id: mask
        width: parent.width
        height: parent.height
        visible: false
        Rectangle {
            x: 0
            y: 0
            width: curSecs * parent.width / totalSecs
            height: 50
        }
    }
    OpacityMask {
        anchors.fill: parent
        source: activeRect
        maskSource: mask
    }
}
