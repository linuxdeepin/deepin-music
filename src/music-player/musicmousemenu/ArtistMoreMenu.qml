// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0

Menu{
    signal viewArtistDatails(var artistData);
    property var artistData
    property string currentPageHash: "artist"

    id: moreMenu
    width: 200
    MenuItem {
        text: qsTr("View details")
        visible: ("artistSublist" === currentPageHash) ? false : true
        height: visible ? 30 : 0
        onTriggered: {
            moreMenu.viewArtistDatails(moreMenu.artistData);
        }
    }
    MenuItem {
        text: qsTr("Play all")
        onTriggered: {
            Presenter.playArtist(artistData.name);
        }
    }
//    MenuItem { text: qsTr("Add to Play queue") }
    MenuSeparator{}
    ImportMenu {
        id:importArtistMenu
        title: qsTr("Add to")
        pageHash: "artist"
        onImportMenuClosed: {
            moreMenu.close();
        }
    }
    onArtistDataChanged: {
        var tmplist = [];
        for(var key in moreMenu.artistData.musicinfos){
            tmplist.push(moreMenu.artistData.musicinfos[key].hash);
        }
        importArtistMenu.mediaHashList = tmplist
    }
}
