// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle {
    property real itemWidth: 2
    property real itemCount: 20
    property bool chromatic: true

    implicitWidth: 200
    implicitHeight: 20
    color: "transparent"

    Rectangle {
        id: greyRect
        width: parent.width
        height: parent.height
        visible: false
        color: "#30000000"
    }

    Rectangle {
        id: gradientRect
        width: parent.width
        height: parent.height
        visible: false
        LinearGradient {
            anchors.fill: parent
            start: Qt.point(0, 0)
            end: Qt.point(parent.width, 0)
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#0206cd"}
                GradientStop { position: 1.0; color: "#3ce6ff"}
            }
        }
    }
    Item {
        id: mask
        width: parent.width
        height: parent.height
        visible: false
        Row {
            width: parent.width
            height: parent.height
            spacing: itemWidth

            Repeater {
                model: itemCount
                Rectangle {
                    width: itemWidth
                    height: parent.height
                    anchors.bottom: parent.bottom
                }
            }
        }
    }
    OpacityMask {
        anchors.fill: parent
        source: chromatic ? gradientRect : greyRect
        maskSource: mask
    }
}
