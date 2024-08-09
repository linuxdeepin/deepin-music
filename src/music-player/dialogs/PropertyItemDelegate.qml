// Copyright (C) 2022 UnionTech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes
import org.deepin.dtk 1.0

Control {
    id: control
    property string title
    property string description
    property var cornersRadius
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
            property Palette backgroundColor: Palette {
                normal: Qt.rgba(0, 0, 0, 0.6)
                normalDark: Qt.rgba(247.0 / 255.0, 247.0 / 255.0, 247.0 / 255.0, 1)
            }
            Layout.leftMargin: 10
            visible: control.title
            text: control.title
            font: DTK.fontManager.t10
            color: ColorSelector.backgroundColor
        }
        RowLayout {
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            Label {
                property Palette textColor: Palette {
                    normal: Qt.rgba(0, 0, 0, 1)
                    normalDark: Qt.rgba(247.0 / 255.0, 247.0 / 255.0, 247.0 / 255.0, 1)
                }
                visible: control.description
                Layout.fillWidth: true
                text: control.description
                font: DTK.fontManager.t7
                elide: Text.ElideMiddle
                color:ColorSelector.textColor
            }
            Loader {
                Layout.leftMargin: 5
                sourceComponent: control.action
            }
        }
    }

    background: Shape {
        id: idShapeControl
        implicitWidth: 66
        implicitHeight: 40
        layer.smooth: true
        ShapePath {
            startX: 0
            startY: cornersRadius[0]
            fillColor: DTK.themeType === ApplicationHelper.LightType ? Qt.rgba(0, 0, 0, 0.05) : Qt.rgba(247, 247, 247, 0.05)
            strokeColor: "transparent"
            strokeWidth: 0
            PathQuad { x: cornersRadius[0]; y: 0; controlX: 0; controlY: 0 }
            PathLine { x: idShapeControl.width - cornersRadius[1]; y: 0 }
            PathQuad { x: idShapeControl.width; y: cornersRadius[1]; controlX: idShapeControl.width; controlY: 0 }
            PathLine { x: idShapeControl.width; y: idShapeControl.height - cornersRadius[2] }
            PathQuad { x: idShapeControl.width - cornersRadius[2]; y: idShapeControl.height; controlX: idShapeControl.width; controlY: idShapeControl.height }
            PathLine { x: cornersRadius[3]; y: idShapeControl.height }
            PathQuad { x: 0; y: idShapeControl.height - cornersRadius[3]; controlX: 0; controlY: idShapeControl.height }
            PathLine { x: 0; y: cornersRadius[0] }
        }
    }
}
