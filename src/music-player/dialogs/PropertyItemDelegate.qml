// Copyright (C) 2022 UnionTech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Control {
    id: control
    property string title
    property string description
    property int corners: RoundRectangle.NoneCorner
    property string iconName
    signal clicked()
    property Component action: ActionButton {
        visible: control.iconName
        Layout.alignment: Qt.AlignRight
        icon {
            width: 14
            height: 14
            name: control.iconName
        }
        onClicked: control.clicked()
    }
    padding: 5
    contentItem: ColumnLayout {
        Label {
            visible: control.title
            text: control.title
            font: DTK.fontManager.t10
            color: Qt.rgba(0, 0, 0, 0.6)
        }
        RowLayout {
            Label {
//                property Palette backgroundColor: Palette {
//                    normal: "#000000"
//                    normal.dark: "#ffffff"
//                }
                visible: control.description
                Layout.fillWidth: true
                text: control.description
                font: DTK.fontManager.t7
//                font.weight: Font.Medium
//                color: ColorSelector.backgroundColor
//                color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(247, 247, 247, 1) : Qt.rgba(0, 0, 0, 1)
                elide: Text.ElideMiddle
            }
            Loader {
                Layout.leftMargin: 5
                sourceComponent: control.action
            }
        }
    }

    background: RoundRectangle {
        implicitWidth: 66
        implicitHeight: 40
        color: Qt.rgba(0, 0, 0, 0.05)
        radius: /*Style.control.radius*/ 8
        corners: control.corners
    }
}
