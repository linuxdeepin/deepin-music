// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import org.deepin.music 1.0

Window {
    id: root
    width: 320
    height: 160
    x: (Screen.width - width) / 2
    y: (Screen.height - height) / 2
    color: "transparent"
    visible: true
    title: qsTr("Music Control")

    // Determine dark mode from the icon theme name reported by DDE Appearance.
    // This avoids a hard dependency on DTK Declarative's QML plugin.
    readonly property bool isDark: MusicApplet.iconThemeKey.indexOf("dark") >= 0

    Rectangle {
        id: card
        anchors.fill: parent
        color: isDark ? Qt.rgba(30/255, 30/255, 30/255, 1) : Qt.rgba(45/255, 45/255, 45/255, 1)
        radius: 8

        ColumnLayout {
            id: mainColumn
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            // Top: Album art + track info
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                // Album art
                Rectangle {
                    id: artBackground
                    width: 48
                    height: 48
                    radius: 4
                    color: isDark ? Qt.rgba(55/255, 55/255, 55/255, 1) : Qt.rgba(61/255, 61/255, 61/255, 1)
                    Layout.alignment: Qt.AlignVCenter
                    clip: true

                    // Fallback icon when no cover art (player absent or track has no mpris:artUrl).
                    // Uses the system music theme icon via image://theme provider (QIcon::fromTheme),
                    // so it follows the active icon theme (bloom / bloom-classic-dark / ...).
                    // The '#' fragment carries iconThemeKey so QtQuick's image cache is busted
                    // and the icon reloads when the system icon theme changes.
                    Image {
                        id: fallbackIcon
                        anchors.centerIn: parent
                        source: "image://theme/deepin-music#" + MusicApplet.iconThemeKey
                        sourceSize.width: 28
                        sourceSize.height: 28
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                        visible: !MusicApplet.musicAvailable || MusicApplet.artSource.toString() === ""
                    }

                    // Album art image
                    Image {
                        id: artImage
                        anchors.fill: parent
                        source: MusicApplet.artSource
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        cache: false
                        visible: MusicApplet.musicAvailable && MusicApplet.artSource.toString() !== ""
                    }
                }

                // Track info
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 2

                    Label {
                        text: MusicApplet.titleText
                        color: isDark ? "#E5ffffff" : "#E5000000"
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        renderType: Text.NativeRendering
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    Label {
                        text: MusicApplet.subtitleText
                        color: "#7C7C7C"
                        font.pixelSize: 11
                        renderType: Text.NativeRendering
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
            }

            // Playback controls
            RowLayout {
                id: controlsRow
                Layout.alignment: Qt.AlignHCenter
                spacing: 16

                // Previous
                AbstractButton {
                    id: prevBtn
                    width: 32
                    height: 32
                    opacity: MusicApplet.canGoPrevious ? 1.0 : 0.4
                    onClicked: MusicApplet.playPreviousTrack()

                    // Theme icon via image provider
                    Image {
                        anchors.centerIn: parent
                        source: "image://theme/toolbar_previous#" + MusicApplet.iconThemeKey
                        sourceSize.width: 16
                        sourceSize.height: 16
                        asynchronous: true
                    }
                }

                // Play/Pause
                AbstractButton {
                    id: playPauseBtn
                    width: 40
                    height: 40
                    opacity: MusicApplet.canTogglePlayback ? 1.0 : 0.4
                    onClicked: MusicApplet.toggleMusicPlayback()

                    Image {
                        anchors.centerIn: parent
                        source: "image://theme/" + (MusicApplet.musicPlaying ? "toolbar_pause" : "toolbar_play") + "#" + MusicApplet.iconThemeKey
                        sourceSize.width: 20
                        sourceSize.height: 20
                        asynchronous: true
                    }
                }

                // Next
                AbstractButton {
                    id: nextBtn
                    width: 32
                    height: 32
                    opacity: MusicApplet.canGoNext ? 1.0 : 0.4
                    onClicked: MusicApplet.playNextTrack()

                    Image {
                        anchors.centerIn: parent
                        source: "image://theme/toolbar_next#" + MusicApplet.iconThemeKey
                        sourceSize.width: 16
                        sourceSize.height: 16
                        asynchronous: true
                    }
                }
            }

            // Open player link
            Label {
                text: qsTr("Open Player")
                color: "#888888"
                font.pixelSize: 10
                renderType: Text.NativeRendering
                Layout.alignment: Qt.AlignHCenter

                MouseArea {
                    anchors.fill: parent
                    anchors.topMargin: -4
                    anchors.bottomMargin: -4
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onClicked: MusicApplet.openMusicPlayer()
                }
            }
        }
    }
}
