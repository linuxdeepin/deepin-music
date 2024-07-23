// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import QtQml.Models 2.11
import org.deepin.dtk 1.0

DialogWindow {
    property var musicData
    property var radius: Style.control.radius

    width: 386
    height: 468
    header: DialogTitleBar {
        enableInWindowBlendBlur: true
        content: Loader {
            sourceComponent: Label {
                anchors.centerIn: parent
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font: DTK.fontManager.t8
                text: musicData === undefined ? " " : musicData.title       //musicTitle
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

        Image {
            Layout.alignment: Qt.AlignHCenter
            sourceSize: Qt.size(118, 118)
            source: musicData === undefined ? " " : "file:///" + musicData.coverUrl  //imgSourcePath
        }
        ColumnLayout {
            spacing: 1
            PropertyItemDelegate {
                Layout.fillWidth: true
                title: qsTr("Title")
                description: musicData === undefined ? " " : musicData.title       //musicTitle
                iconName: "action_edit"
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
