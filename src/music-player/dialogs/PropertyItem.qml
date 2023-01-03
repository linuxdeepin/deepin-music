// Copyright (C) 2022 UnionTech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
