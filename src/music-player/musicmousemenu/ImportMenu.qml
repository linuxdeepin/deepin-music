// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import QtQml.Models 2.11
import org.deepin.dtk 1.0

Menu {
    id: importMenu

    property int itemIndex: -1  //判断当前菜单属于哪个列表项，用于控制菜单按钮的显隐

    property var mediaHashList: []
    property string pageHash: ""

    signal importMenuClosed

    width: 200

    MenuItem {
        height: visible ? 30 : 0
        text: qsTr("Play queue")
        visible: ("play" === pageHash) ? false : true

        onTriggered: {
            Presenter.addMetasToPlayList(mediaHashList, "play");
        }
    }

    MenuSeparator {
        height: visible ? 12 : 0
        visible: ("play" === pageHash) ? false : true
    }

    MenuItem {
        height: visible ? 30 : 0
        text: qsTr("My favorites")
        visible: ("fav" === pageHash) ? false : true

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

        height: visible ? 12 : 0
        visible: (globalVariant.globalCustomPlaylistModel.tmpModel.count === 0) ? false : true
    }

    Repeater {
        id: repeater

        model: globalVariant.globalCustomPlaylistModel.tmpModel

        MenuItem {
            height: visible ? 30 : 0
            text: model.displayName.replace(/</g, "&lt;")
            visible: (model.uuid === pageHash) ? false : true

            onTriggered: {
                Presenter.addMetasToPlayList(mediaHashList, model.uuid);
                importMenu.importMenuClosed();
                close();
            }
        }
    }
}
