// SPDX-FileCopyrightText: 2023-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0
import "../dialogs"

Menu {
    property string pageHash: ""
    property var musicHashList: []

    id: mulitSelectMenu
    width: 200

    ImportMenu {
        title: qsTr("Add to")
        pageHash: mulitSelectMenu.pageHash
        mediaHashList: mulitSelectMenu.musicHashList
        onImportMenuClosed: {
            mulitSelectMenu.close();
        }
    }
    DeleteSonglistDialog {id: removeSong; listHash: pageHash; deleteHashList: musicHashList}
    MenuItem {
        text: pageHash === "play" ? qsTr("Remove from play queue") : qsTr("Remove from playlist")
        Accessible.name: "MulitSelectMenu_Play"
        onTriggered: {
            removeSong.show();
        }
    }
    DeleteLocalDialog {id: deleteLocal; listHash: pageHash; deleteHashList: musicHashList}
    MenuItem {
        text: qsTr("Delete from local disk")
        Accessible.name: "MulitSelectMenu_DeleteFromLocalDisk"
        onTriggered: {
            deleteLocal.show();
        }
    }
}
