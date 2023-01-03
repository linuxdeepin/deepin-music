// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import QtQml.Models 2.11
import org.deepin.dtk 1.0

Menu {
    signal viewAlbumDatails(var albumData);
    property var albumData
    property string currentPageHash: "album"

    id: moreMenu
    width: 200

    MenuItem {
        text: qsTr("View details")
        visible: ("albumSublist" === currentPageHash) ? false : true
        height: visible ? 30 : 0
        onTriggered: {
            moreMenu.viewAlbumDatails(moreMenu.albumData);
        }
    }
    MenuItem {
        text: qsTr("Play all")
        onTriggered: {
            Presenter.playAlbum(albumData.name);
        }
    }
//    MenuItem { text: qsTr("Add to Play queue") }
    MenuSeparator{}
    ImportMenu {
        id:imporAlbumtMenu
        title: qsTr("Add to")
        pageHash: "album"
        onImportMenuClosed: {
            moreMenu.close();
        }
    }
    onAlbumDataChanged: {
        var tmplist = [];
        for(var key in moreMenu.albumData.musicinfos){
            tmplist.push(moreMenu.albumData.musicinfos[key].hash);
        }
        imporAlbumtMenu.mediaHashList = tmplist
    }
}
