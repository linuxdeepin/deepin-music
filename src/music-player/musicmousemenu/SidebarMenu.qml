// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0
import "../dialogs"
Menu{
    property string pageHash: ""

    id: playlistMenu
    width: 200
    MenuItem {
        text: qsTr("Play")
        onTriggered: {
            Presenter.playPlaylist(playlistMenu.pageHash);
        }
    }
    MenuSeparator {}
    MenuItem {
        text: qsTr("Add songs");
        onTriggered: {
            globalVariant.curListPage = playlistMenu.pageHash
            globalVariant.globalFileDlgOpen()
        }
    }
    MenuItem {
        visible: (pageHash === "fav") ? false : true
        height: visible ? 36: 0
        text: qsTr("Rename")
        onTriggered: {
            globalVariant.renamePlaylist(playlistMenu.pageHash);
        }
    }
    DeleteSonglistDialog{
        id: removeSong
        listHash: playlistMenu.pageHash
        removeMusic: false
    }
    MenuItem {
        id: deletePlaylist
        visible: (pageHash === "fav") ? false : true
        height: visible ? 36: 0
        text: qsTr("Delete");
        onTriggered: {
            removeSong.show();
        }
    }
}
