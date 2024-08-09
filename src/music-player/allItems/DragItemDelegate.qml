// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import Qt5Compat.GraphicalEffects

Rectangle {
    property string url: ""

    id: root
    anchors.centerIn: parent
    width: parent.width - 20
    height: parent.height - 20
    radius: 5
    color: "#aaaaaaaa"
    antialiasing: true
    smooth: true

    Image {
        id: img
        anchors.centerIn: parent
        width: parent.width - 4 //减掉4个像素显示边框
        height: parent.height - 4
        visible: false

        source: "file:///" + url
        fillMode: Image.PreserveAspectFit
    }
    Rectangle {
        id: mask
        anchors.fill: img
        radius: 5
        visible: false
    }
    OpacityMask {
        anchors.fill: img
        source: img
        maskSource: mask
    }
}
