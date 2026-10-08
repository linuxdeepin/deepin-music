// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0

Button {
    property string iconName: "details_play"
    property Palette backgroundColor: Palette {
        normal: Qt.rgba(255, 255, 255, 0.5)
        hovered: Qt.rgba(255, 255, 255, 0.55)
    }
    id: control
    width: 40
    height: 40
    background: Rectangle {
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
        radius: width / 2
        color: control.ColorSelector.backgroundColor
        BoxShadow {
            anchors.fill: parent
            shadowOffsetX: 0
            shadowOffsetY: 2
            shadowColor: /*control.D.ColorSelector.dropShadowColor*/Qt.rgba(0, 0, 0, 0.2)
            shadowBlur: 4
            cornerRadius: parent.radius
            spread: 0
            hollow: true
    }

        }
    contentItem: DciIcon {
        name: iconName
        anchors.centerIn: parent
    }
}
