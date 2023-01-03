// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import "../dialogs"

ListModel {
    id: customPlaylistModel
    property ListModel tmpModel: ListModel {}

    function loadMediaDatas() {
        customPlaylistModel.clear();
        tmpModel.clear();
        var favourite = {};
        favourite.checked = false;
        favourite.icon = "like";
        favourite.icon_checked = "like_checked";
        favourite.displayName = qsTr("My Favorites")
        favourite.uuid = "fav";
        favourite.editable = false;
        favourite.dragFlag = false;

        customPlaylistModel.append(favourite);

        var tempModel = Presenter.customPlaylistInfos();
        for(var i = 0; i < tempModel.length; i++){
            var tmpPlaylist = tempModel[i];
            tmpPlaylist.checked = false;
            tmpPlaylist.icon = "list";
            tmpPlaylist.icon_checked = "list_checked";
            tmpPlaylist.editable = true;
            tmpPlaylist.dragFlag = false;
            customPlaylistModel.append(tmpPlaylist);
            tmpModel.append(tmpPlaylist);
        }
    }
    function onRenamedPlaylist(name, playlistHash){
        for(var j = customPlaylistModel.count - 1; j >= 0; j--){
            if(customPlaylistModel.get(j).uuid === playlistHash){
                customPlaylistModel.setProperty(j, "displayName", name);
                tmpModel.setProperty(j - 1, "displayName", name);
                break;
            }
        }
        globalVariant.updateCurrentPlaylistTitleName(name, playlistHash);
    }
    function onDeletedPlaylist(playlistHash){
        for(var k = customPlaylistModel.count - 1; k >= 0; k--){
            if(customPlaylistModel.get(k).uuid === playlistHash){
                if (playlistHash === "cdarole")
                    globalVariant.cdRemovedDlgShow()

                customPlaylistModel.remove(k);
                tmpModel.remove(k - 1);
                globalVariant.switchToPreviousPlaylist(k - 1);
                break;
            }
        }
    }
    function onAddPlaylist(playlist){
        playlist.checked = false;
        playlist.icon = "list";
        playlist.editable = true;
        playlist.dragFlag = false;
        customPlaylistModel.append(playlist);
        tmpModel.append(playlist);
    }
    function onCDStatusChanged(status) {
        if (status > 0) {
            var cdList = {};
            cdList.checked = false;
            cdList.icon = "list";
            cdList.displayName = qsTr("CD playlist")
            cdList.uuid = "cdarole";
            cdList.editable = false;
            cdList.dragFlag = false;

            customPlaylistModel.insert(1, cdList)
            globalVariant.sendFloatingMessageBox(qsTr("CD"), 3);
        }
    }


    Component.onCompleted: {
        customPlaylistModel.loadMediaDatas();
        Presenter.renamedPlaylist.connect(onRenamedPlaylist);
        Presenter.deletedPlaylist.connect(onDeletedPlaylist);
//        Presenter.addedPlaylist.connect(onAddPlaylist);
        Presenter.updateCDStatus.connect(onCDStatusChanged);
    }
}
