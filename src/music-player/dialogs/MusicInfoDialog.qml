// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQml 2.15
import QtQuick.Layouts
import QtQuick.Window
import QtQml.Models
import org.deepin.dtk 1.0

DialogWindow {
    property var musicData
    property var radius: Style.control.radius

    width: 386
    height: 468
    Binding on flags {
        when: Qt.platform.os === "windows"
        value: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.MSWindowsFixedSizeDialogHint | Qt.FramelessWindowHint
    }

    onVisibleChanged: {
        if (visible && Qt.platform.os === "windows") {
            x = (Screen.width - width) / 2
            y = (Screen.height - height) / 2
        }
    }

    header: DialogTitleBar {
        enableInWindowBlendBlur: false
        content: Loader {
            sourceComponent: Label {
                property Palette textColor: Palette {
                    normal: Qt.rgba(0, 0, 0, 1)
                    normalDark: Qt.rgba(247.0 / 255.0, 247.0 / 255.0, 247.0 / 255.0, 1)
                }
                anchors.centerIn: parent
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font: DTK.fontManager.t7
                text: musicData === undefined ? " " : musicData.title       //musicTitle
                color:ColorSelector.textColor
            }
        }
    }
    modality: Qt.ApplicationModal

    ColumnLayout {
        id: content
        anchors {
            left: parent.left
            horizontalCenter: parent.horizontalCenter
            margins: 10
        }

        Rectangle {
            id: imageRect
            width: 118
            height: 118
            Layout.alignment: Qt.AlignHCenter
            BoxShadow {
                anchors.fill: musicImage
                shadowOffsetX: 0
                shadowOffsetY: 10
                shadowColor: Qt.rgba(0, 0, 0, 0.2)
                shadowBlur: 20
            }

            Image {
                id: musicImage
                Layout.alignment: Qt.AlignHCenter
                sourceSize: Qt.size(118, 118)
                source: musicData === undefined || !musicData.coverUrl ? "" : "file:///" + musicData.coverUrl
                cache: false
            }
        }

        ColumnLayout {
            Layout.topMargin: 10
            spacing: 1
            PropertyItemDelegate {
                Layout.fillWidth: true
                title: qsTr("Title")
                description: musicData === undefined ? " " : musicData.title       //musicTitle
                // TODO(pengfeixx): The function is not implemented, and the entrance is temporarily removed
                // iconName: "action_edit"
                cornersRadius: [radius, radius, 0, 0]
            }

            PropertyItemDelegate {
                Layout.fillWidth: true
                title: qsTr("Artist")
                description: musicData === undefined ? " " : musicData.artist    //musicArtist
                cornersRadius: [0, 0, 0, 0]
            }
            PropertyItemDelegate {
                Layout.fillWidth: true
                title: qsTr("Album")
                description: musicData === undefined ? " " : musicData.album       //musicAlbum
                cornersRadius: [0, 0, radius, radius]
            }
        }
        RowLayout {
            spacing: 1
            PropertyItemDelegate {
                Layout.fillWidth: true
                title: qsTr("Type")
                description: musicData === undefined ? " " : musicData.filetype      //fileType
                cornersRadius: [8, 0, 0, radius]
            }
            PropertyItemDelegate {
                Layout.fillWidth: true
                title: qsTr("Size")
                description: "%1M".arg((musicData === undefined ? 0 : musicData.size / 1024 / 1024).toFixed(2))
                cornersRadius: [0, 0, 0, 0]
            }
            PropertyItemDelegate {
                Layout.fillWidth: true
                title: qsTr("Duration")
                description: musicData === undefined ? "0:00" : Math.floor(musicData.length / 1000 / 60) + ":" + Math.floor(musicData.length / 1000 % 60)
                cornersRadius: [0, radius, radius, 0]
            }
        }
        PropertyItemDelegate {
            Layout.fillWidth: true
            title: qsTr("Path")
            description: musicData === undefined ? " " : musicData.localPath       //filePath
            cornersRadius: [radius, radius, radius, radius]
        }
    }
}
