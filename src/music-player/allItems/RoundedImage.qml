// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtGraphicalEffects 1.0
import org.deepin.dtk 1.0

Rectangle {
    property url backgroundImageUrl
    property int backgroundWidth
    property int backgroundHeight
    property bool isCurPlay: false
    property bool isCurHover: false

    id: backgroundImg
    anchors.centerIn: parent
    width: backgroundWidth; height: backgroundHeight
    color: "transparent"
    smooth: true
    Image {
        id: image
        source: backgroundImageUrl
        width: backgroundWidth; height: backgroundHeight
//        sourceSize: Qt.size(backgroundWidth, backgroundHeight)
        visible: false
        smooth: true
        antialiasing: true
        Rectangle {
            anchors.fill: parent
            color: isCurPlay || isCurHover ? Qt.rgba(0, 0, 0, 0.3) : Qt.rgba(0, 0, 0, 0)
        }
    }
    Rectangle {
        id: mask
        anchors.fill: image
        radius: 8
        visible: false
    }
    OpacityMask{
        id: opacityMask
        anchors.fill: image
        source: image
        maskSource: mask
    }
    FastBlur {
        anchors.top: opacityMask.top; anchors.topMargin: 6
        anchors.left: opacityMask.left; anchors.leftMargin: 1
        width: opacityMask.width - 2; height: opacityMask.width - 6
        source: opacityMask
        radius: 24
        transparentBorder: true
    }
    OpacityMask {
        id:mask_image
        anchors.fill: image
        source: image
        maskSource: mask
        antialiasing: true
        smooth: true
     }

    //border and shadow
    Rectangle {
        id: borderRect
        anchors.fill: parent
        color: "transparent"
        border.color: Qt.rgba(0, 0, 0, 0.06)
        border.width: 1
        visible: true
        radius: 8
    }
    BoxShadow {
        anchors.fill: parent
        shadowOffsetX: 0
        shadowOffsetY: 4
        shadowColor: Qt.rgba(0, 0, 0, 0.06)
        shadowBlur: 10
        cornerRadius: 8
        spread: 0
        hollow: true
    }
}
