// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0
import "../allItems"

Item {
    id: root

    property bool favorite: false
    property bool selected: false
    property var controlPalette
    signal clicked()

    width: 32
    height: 32

    ActionButton {
        anchors.fill: parent
        visible: root.favorite
        hoverEnabled: false
        icon.name: "heart_check"
        icon.width: 16
        icon.height: 16
        palette.windowText: "#F75B5B"
    }

    DciIcon {
        id: favoriteIcon
        anchors.centerIn: parent
        width: 16
        height: 16
        sourceSize: Qt.size(16, 16)
        theme: DTK.themeType
        palette: DTK.makeIconPalette(root.controlPalette)
        visible: false
        name: "heart"
    }

    ColorOverlay {
        anchors.fill: favoriteIcon
        source: favoriteIcon
        color: root.selected
               ? root.controlPalette.highlightedText
               : root.controlPalette.windowText
        visible: !root.favorite
        cached: true
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
