// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import QtQml.Models 2.11
import org.deepin.dtk 1.0

Menu{
    property string pageHash: ""
    property var mediaHashList: []
    signal importMenuClosed();

    id: importMenu
    width: 200
    MenuItem {
        text: qsTr("Play queue")
        visible: ("play" === pageHash) ? false : true
        height: visible ? 30 : 0
        onTriggered: {
            Presenter.addMetasToPlayList(mediaHashList, "play");
        }
    }
    MenuSeparator {
        visible: ("play" === pageHash) ? false : true
        height: visible ? 12 : 0
    }
    MenuItem {
        text: qsTr("My favorites")
        visible: ("fav" === pageHash) ? false : true
        height: visible ? 30 : 0
        onTriggered: {
            Presenter.addMetasToPlayList(mediaHashList, "fav");
        }
    }
    MenuItem {
        text: qsTr("Create new playlist")
        onTriggered: {
            var tmpPlaylist = Presenter.addPlayList(qsTr("New playlist"));
            globalVariant.globalCustomPlaylistModel.onAddPlaylist(tmpPlaylist);
            Presenter.addMetasToPlayList(mediaHashList, tmpPlaylist.uuid);
            globalVariant.renameNewItem();
        }
    }
    MenuSeparator {
        id: menuSeparator
        visible: (globalVariant.globalCustomPlaylistModel.tmpModel.count === 0) ? false : true;
        height: visible ? 12 : 0;
    }
    Repeater {
        id: repeater
        model: globalVariant.globalCustomPlaylistModel.tmpModel
        MenuItem {
            text: model.displayName.replace(/</g, "&lt;")
            visible: (model.uuid === pageHash) ? false : true
            height: visible ? 30 : 0
            onTriggered: {
                Presenter.addMetasToPlayList(mediaHashList, model.uuid);
                importMenu.importMenuClosed();
                close();
            }
        }
    }
}



