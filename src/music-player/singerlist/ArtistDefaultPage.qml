// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0

import "../dialogs"

Rectangle {
    property FileDialog fileDlg: FileDialog{}
    property FolderDialog folderDlg: FolderDialog{}
    color: Qt.rgba(0, 0, 0, 0)

    Rectangle {
        id: defaultRectangle
        anchors.centerIn: parent
        anchors.top: parent.top; anchors.topMargin: 107
        color: Qt.rgba(0, 0, 0, 0)
        width: 376; height: 242
        Image {
            id: albumImage
            anchors.horizontalCenter: defaultRectangle.horizontalCenter;
            source: "qrc:/dsg/img/artist_default.png"
            sourceSize: Qt.size(126, 126);
        }
        Row{
            id: buttonRow
            anchors.top: albumImage.bottom; anchors.topMargin: 40
            spacing: 20
            Button {
                id: importMusicFile
                text: qsTr("Add Songs");
                width: 178; height: 36
                onClicked: {
                    fileDlg.open()
                }
            }
            Button {
                id: importMusicDir
                text: qsTr("Open Folders");
                width: 178; height: 36
                onClicked: {
                    folderDlg.open()
                }
            }
        }

        Row {
            anchors.top: buttonRow.bottom; anchors.topMargin: 20
            anchors.horizontalCenter: defaultRectangle.horizontalCenter
            Label {
                id: lable
                height: 20
                color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(247, 247, 247, 0.7)
                                                                    : Qt.rgba(0, 0, 0, 0.4)
                text: qsTr("Drag music files here or");
            }
            Label {
                height: 20;
                text: qsTr(" scan the Music directory");
                color: palette.highlight
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    onClicked: {
                        Presenter.importMetas(null,"artist");
                    }
                }
            }
        }

    }
}
