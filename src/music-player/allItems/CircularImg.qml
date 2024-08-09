// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS
import audio.global 1.0

Rectangle {
    property url cuted_background
    property int background_width
    property int background_height
    property bool isCurPlay: false
    property bool isCurHover: false

    ColorAnimation {
        id: maskChangeAnimator
        target: _imgMask
        property: "color"
        from: Qt.rgba(0, 0, 0, 0)
        to: Qt.rgba(0, 0, 0, 0.3)
        duration: 100
    }

    id: imgRect
    anchors.centerIn: parent
    width: background_width
    height: background_width
    radius: width / 2
    color: "transparent"
    smooth: true

    Image {
        id: _image
        smooth: true
        visible: false
        anchors.fill: parent
        source: cuted_background
//        sourceSize: Qt.size(parent.size, parent.size)
        antialiasing: true
        Rectangle {
            id: _imgMask
            anchors.fill: parent
            color: isCurPlay ? Qt.rgba(0, 0, 0, 0.3) : Qt.rgba(0, 0, 0, 0)
        }
    }
    Rectangle {
        id: _mask
        color: "black"
        anchors.fill: parent
        radius: width/2
        visible: false
        antialiasing: true
        smooth: true
    }
    OpacityMask {
        id:mask_image1
        anchors.fill: _image
        source: _image
        maskSource: _mask
        antialiasing: true
        smooth: true
    }
    FastBlur {
        anchors.top: mask_image1.top; anchors.topMargin: 3
        anchors.left: mask_image1.left; /*anchors.leftMargin: 1*/
        width: mask_image1.width; height: mask_image1.width - 2
        source: mask_image1
        radius: 32
        transparentBorder: true
    }

    OpacityMask {
        id:mask_image
        anchors.fill: _image
        source: _image
        maskSource: _mask
        antialiasing: true
        smooth: true
    }

    Rectangle {
        id: borderRect
        anchors.fill: parent
        color: "transparent"
        border.color: Qt.rgba(0, 0, 0, 0.06)
        border.width: 1
        visible: true
        radius: width / 2
    }
//    BoxShadow {
//        anchors.fill: parent
//        shadowOffsetX: 0
//        shadowOffsetY: 4
//        shadowColor: Qt.rgba(255, 255, 255, 0.5)
//        shadowBlur: 10
//        cornerRadius: imgRect.radius
//        spread: 0
//        hollow: true
//    }
    BoxShadow {
        anchors.fill: parent
        shadowOffsetX: 0
        shadowOffsetY: 4
        shadowColor: Qt.rgba(0, 0, 0, 0.06)
        shadowBlur: 10
        cornerRadius: imgRect.radius
        spread: 0
        hollow: true
    }

    onIsCurHoverChanged: {
        if (isCurPlay)
            return;
        if (isCurHover) {
            maskChangeAnimator.start()
        } else {
            if (maskChangeAnimator.running) {
                maskChangeAnimator.stop()
            }
            _imgMask.color = Qt.rgba(0, 0, 0, 0)
        }
    }
}
