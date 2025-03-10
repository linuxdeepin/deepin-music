// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import "../musicmousemenu"

Rectangle {
    signal musicBaselistChanged(string type, string displayName)

    width: 220
    height: parent.height
    color: Qt.rgba(0, 0, 0, 0)
    ScrollView {
        id: musicBaseScrollView
        clip: true
        focus: true
        width: 220
        height: 560

        ColumnLayout {
            id: colum
            anchors.fill: parent
            spacing: 30
            ButtonGroup { id: paneListGroup }
            SideBarItem{
                id: musicbaseSidebar
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 15
                height: 128
                view.height: 108
                title: qsTr("Library")
                type: "library"
                group: paneListGroup
                sideModel: ListModel {
                    ListElement{checked: false; icon: "album"; icon_checked: "album_checked"; displayName: qsTr("Albums"); uuid: "album"; editable: false; dragFlag: false}
                    ListElement{checked: false; icon: "singer"; icon_checked: "singer_checked"; displayName: qsTr("Artists"); uuid: "artist"; editable: false; dragFlag: false}
                    ListElement{checked: false; icon: "all_music"; icon_checked: "all_music_checked"; displayName: qsTr("All Songs"); uuid: "all"; editable: false; dragFlag: false}
                }
            }
            SideBarItem {
                id: musicSonglist
                title: qsTr("Playlists")
                type: "playlists"
                group: paneListGroup
                fillHeight: true
                sideModel: globalVariant.globalCustomPlaylistModel //全局自定义歌单model
                action: FloatingButton {
                    width: 21; height: 21
                    checked: false

                    DciIcon {
                        name: "sidebar_list_add"
                        sourceSize: Qt.size(15, 15)
                        anchors.centerIn: parent
                        palette: DTK.makeIconPalette(parent.palette)
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Create playlist")
                    onClicked: {
                        var tmpPlaylist = Presenter.addPlayList(qsTr("New playlist"));
                        globalVariant.globalCustomPlaylistModel.onAddPlaylist(tmpPlaylist);
                        musicSonglist.view.onRenameNewItem();
                    }
                }
                Component.onCompleted: {
                    globalVariant.renamePlaylist.connect(musicSonglist.view.onRenamePlaylist);
                    globalVariant.renameNewItem.connect(musicSonglist.view.onRenameNewItem);
                    globalVariant.switchToPreviousPlaylist.connect(musicSonglist.view.onSwitchToPreviousPlaylist);
                }
            }
        }

        Loader { id: sidebarMenuLoader }

        Connections {
            target: musicbaseSidebar
            onItemClicked: {
                musicBaselistChanged(key, text);
            }
            onItemRightClicked: {
                if (sidebarMenuLoader.status === Loader.Null) {
                    sidebarMenuLoader.setSource("../musicmousemenu/SidebarMenu.qml")
                }
                if (sidebarMenuLoader.status === Loader.Ready) {
                    sidebarMenuLoader.item.pageHash = key;
                    sidebarMenuLoader.item.updateMenuState();
                    sidebarMenuLoader.item.popup();
                }
            }
        }
        Connections {
            target: musicSonglist
            onItemClicked: {
                musicBaselistChanged(key, text);
            }
            onItemRightClicked:{
                if (key === "cdarole")
                    return
                if (sidebarMenuLoader.status === Loader.Null )
                    sidebarMenuLoader.setSource("../musicmousemenu/SidebarMenu.qml")
                if (sidebarMenuLoader.status === Loader.Ready ) {
                    sidebarMenuLoader.item.pageHash = key;
                    sidebarMenuLoader.item.updateMenuState();
                    sidebarMenuLoader.item.popup();
                }
            }
        }
    }
}



