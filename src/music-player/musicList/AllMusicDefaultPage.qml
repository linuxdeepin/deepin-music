// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import "../dialogs"

Rectangle {
    property string pageHash: ""
//    property FileDialog fileDlg: FileDialog{}
    property FolderDialog folderDlg: FolderDialog{}
    color: "transparent"
    StackView {
        id: defaultStackview
        anchors.centerIn: parent
        anchors.top: parent.top; anchors.topMargin: 107
        width: 376; height: 242
    }

    Rectangle {
        id: allmusicDefault
        anchors.centerIn: parent
        anchors.top: parent.top; anchors.topMargin: 107
        color: "transparent"
        width: 376; height: 242
        visible: false;
        Image {
            id: albumImage
            anchors.horizontalCenter: allmusicDefault.horizontalCenter;
            source: "qrc:/dsg/img/allMusic_default.png"
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
                    globalVariant.globalFileDlgOpen()
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
            anchors.horizontalCenter: allmusicDefault.horizontalCenter
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
                        Presenter.importMetas({}, pageHash);
                    }
                }
            }
        }
    }

    Rectangle {
        id: favmusicDefault
        color: "transparent"
        width: 376; height: 242
        visible: false;
        Image {
            id: favouriteImage
            anchors.horizontalCenter: favmusicDefault.horizontalCenter;
            source: "qrc:/dsg/img/favourite_default.png"
            sourceSize: Qt.size(126, 126);
        }
        Label {
            id: favouriteLable
            anchors.top: favouriteImage.bottom; anchors.topMargin: 40
            anchors.horizontalCenter: favmusicDefault.horizontalCenter;
            color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(247, 247, 247, 0.7)
                                                                : Qt.rgba(0, 0, 0, 0.4)
            text: qsTr("Your favorite songs will be displayed here");
            horizontalAlignment: Qt.AlignHCenter
        }
        Rectangle {
            width: childrenRect.width
            height: childrenRect.height
            anchors.top: favouriteLable.bottom; anchors.topMargin: 13
            anchors.horizontalCenter: parent.horizontalCenter
            color: "transparent"

            Row {
                Text {
                    text: qsTr("Click “")
                    color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(247, 247, 247, 0.7)
                                                                        : Qt.rgba(0, 0, 0, 0.4)
                }
                DciIcon {
                    name: "default_heart"
                    sourceSize: Qt.size(20, 20)
                    anchors.verticalCenter: parent.verticalCenter
                    theme: DTK.themeType
                }
                Text {
                    text: qsTr("” to add a song to My Favorites")
                    color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(247, 247, 247, 0.7)
                                                                        : Qt.rgba(0, 0, 0, 0.4)
                }
            }
        }

//        Label {
//            anchors.top: favouriteLable.bottom; anchors.topMargin: 13
//            anchors.horizontalCenter: favmusicDefault.horizontalCenter;
//            color: Qt.rgba(0, 0, 0, 0.4)
//            text: qsTr("Click “ ♥ ” to add a song to My Favorites");
//            horizontalAlignment: Qt.AlignHCenter
//        }
    }

    Rectangle {
        id: songListDefault
        color: "transparent"
        width: 376; height: 242
        visible: false;
        Image {
            id: songListImage
            anchors.horizontalCenter: songListDefault.horizontalCenter;
            source: "qrc:/dsg/img/favourite_default.png"
            sourceSize: Qt.size(126, 126);
        }
        Rectangle {
            width: childrenRect.width
            height: childrenRect.height
            anchors.top: songListImage.bottom; anchors.topMargin: 40
            anchors.horizontalCenter: songListDefault.horizontalCenter
            color: Qt.rgba(0, 0, 0, 0)
            Row {
                Text {
                    text: qsTr("No songs yet. Click “")
                    color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(247, 247, 247, 0.7)
                                                                        : Qt.rgba(0, 0, 0, 0.4)
                }
                DciIcon {
                    name: "default_add"
                    sourceSize: Qt.size(20, 20)
                    anchors.verticalCenter: parent.verticalCenter
                    theme: DTK.themeType
                }
                Text {
                    text: qsTr("” to add songs to the playlist")
                    color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(247, 247, 247, 0.7)
                                                                        : Qt.rgba(0, 0, 0, 0.4)
                }
            }
        }

//        Label {
//            id: songListLable
//            anchors.top: songListImage.bottom; anchors.topMargin: 40
//            anchors.horizontalCenter: songListDefault.horizontalCenter;
//            color: Qt.rgba(0, 0, 0, 0.4)
//            text: qsTr("No songs yet. Click “ ➕ ” to add songs to the playlist.")
//            horizontalAlignment: Qt.AlignHCenter
//        }
    }
    onPageHashChanged: {
        if(pageHash === "all"){
            defaultStackview.push(allmusicDefault);
        }else if(pageHash === "fav"){
            defaultStackview.push(favmusicDefault);
        }else{
            defaultStackview.push(songListDefault);
        }
    }

}
