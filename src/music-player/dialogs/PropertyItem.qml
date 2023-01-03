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
import QtQml.Models 2.11
import org.deepin.dtk 1.0

ColumnLayout {
    width: parent.width
    property string title
    default property alias content: itemModel.children

    ItemDelegate {
        id: titleBar
        Layout.fillWidth: true; Layout.preferredHeight: 24
        text: title
        icon.name: info.visible ? "go-up": "go-down"
        display: IconLabel.IconBesideText
        checkable: false
        font: DTK.fontManager.t5
        backgroundVisible: false
        MouseArea {
            anchors.fill: parent
            onClicked: {
                info.visible = !info.visible
            }
        }
    }
    ListView {
        id: info
        Layout.fillWidth: true
        Layout.preferredHeight: contentHeight
        spacing: 5
        model: ObjectModel {
            id: itemModel
        }
        Component.onCompleted: {
            for (var i = 0; i < count; ++i) {
                var item = model.get(i)
                item.width = width
            }
        }
    }
}
