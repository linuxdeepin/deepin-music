/*
 * Copyright (C) 2022 UnionTech Technology Co., Ltd.
 *
 * Author:     yeshanshan <yeshanshan@uniontech.com>
 *
 * Maintainer: yeshanshan <yeshanshan@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
