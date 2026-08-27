// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.deepin.dtk 1.0 as D

Item {
    id: root
    implicitWidth: 280
    implicitHeight: 40

    readonly property bool dark: D.DTK.themeType === D.ApplicationHelper.DarkType
    readonly property int valueSize: Math.max(12, Math.min(20, Math.round(height * 0.2)))
    readonly property int labelSize: Math.max(8, Math.round(valueSize * 0.45))
    readonly property int iconSize: Math.round(valueSize * 1.6)

    readonly property color colorText: dark ? Qt.rgba(1, 1, 1, 0.96) : Qt.rgba(0, 0, 0, 0.92)
    readonly property color colorSecondary: dark ? Qt.rgba(1, 1, 1, 0.68) : Qt.rgba(0, 0, 0, 0.58)

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
        propagateComposedEvents: true
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 6
        anchors.rightMargin: 6
        spacing: 6

        Item {
            width: root.iconSize
            height: root.iconSize
            Layout.alignment: Qt.AlignVCenter

            D.DciIcon {
                name: "deepin-music"
                sourceSize: Qt.size(root.iconSize, root.iconSize)
                palette: D.DTK.makeIconPalette(root.palette)
                theme: dark ? D.DciIcon.Dark : D.DciIcon.Light
                anchors.fill: parent
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: musicApplet.openMusicPlayer()
            }
        }

        // ── Track info (visible when NOT hovered) ─────────────────
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 2
            visible: !hoverArea.containsMouse
            opacity: hoverArea.containsMouse ? 0.0 : 1.0
            Behavior on opacity { NumberAnimation { duration: 120 } }

            Text {
                text: musicApplet.titleText
                color: root.colorText
                font.pixelSize: root.valueSize
                font.weight: Font.DemiBold
                renderType: Text.NativeRendering
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Text {
                visible: musicApplet.musicAvailable && musicApplet.subtitleText !== ""
                text: musicApplet.subtitleText
                color: root.colorSecondary
                font.pixelSize: root.labelSize
                renderType: Text.NativeRendering
                elide: Text.ElideRight
                Layout.maximumWidth: parent.width * 0.4
            }
        }

        // ── Playback controls (visible when hovered) ──────────────
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 8
            visible: hoverArea.containsMouse
            opacity: hoverArea.containsMouse ? 1.0 : 0.0
            Behavior on opacity { NumberAnimation { duration: 120 } }

            Item { Layout.fillWidth: true }

            // Previous — no system DCI, falls back to QIcon::fromTheme
            Item {
                width: root.iconSize
                height: root.iconSize
                opacity: musicApplet.canGoPrevious ? 1.0 : 0.35

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: musicApplet.playPreviousTrack()
                }

                D.DciIcon {
                    name: "media-skip-forward"
                    fallbackToQIcon: true
                    sourceSize: Qt.size(root.iconSize, root.iconSize)
                    palette: D.DTK.makeIconPalette(root.palette)
                    theme: dark ? D.DciIcon.Dark : D.DciIcon.Light
                    anchors.centerIn: parent
                }
            }

            // Play / Pause
            Item {
                width: root.iconSize
                height: root.iconSize
                opacity: musicApplet.canTogglePlayback ? 1.0 : 0.35

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: musicApplet.toggleMusicPlayback()
                }

                D.DciIcon {
                    name: musicApplet.musicPlaying
                           ? "media-playback-pause"
                           : "media-playback-start"
                    fallbackToQIcon: true
                    sourceSize: Qt.size(root.iconSize, root.iconSize)
                    palette: D.DTK.makeIconPalette(root.palette)
                    theme: dark ? D.DciIcon.Dark : D.DciIcon.Light
                    anchors.centerIn: parent
                }
            }

            // Next
            Item {
                width: root.iconSize
                height: root.iconSize
                opacity: musicApplet.canGoNext ? 1.0 : 0.35

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: musicApplet.playNextTrack()
                }

                D.DciIcon {
                    name: "media-skip-backward"
                    fallbackToQIcon: true
                    sourceSize: Qt.size(root.iconSize, root.iconSize)
                    palette: D.DTK.makeIconPalette(root.palette)
                    theme: dark ? D.DciIcon.Dark : D.DciIcon.Light
                    anchors.centerIn: parent
                }
            }

            Item { Layout.fillWidth: true }
        }
    }
}
