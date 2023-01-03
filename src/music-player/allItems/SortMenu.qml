// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk 1.0
import audio.global 1.0

ToolButton{
    property string sortPageHash: "all"
    property int pageSortType: DmGlobal.SortByNull
    property string pageTitle: ""

    id: sortBtn
    width: 54; height: 36
    anchors.right: parent.right
    anchors.verticalCenter: parent.verticalCenter
    indicator: ButtonIndicator {}
    icon.name: {
        if (sortPageHash === "artist" || sortPageHash === "artistResult") {
            return artistmodel.get(pageSortType).iconName
        } else if (sortPageHash === "album" || sortPageHash === "albumResult") {
            return albummodel.get(pageSortType).iconName
        } else {
            return listmodel.get(pageSortType).iconName
        }
    }

    enabled: pageTitle === qsTr("CD playlist") ? false : true

    ListModel {
        id: listmodel
        ListElement { title: qsTr("Custom"); iconName: "view_list" }
        ListElement { title: qsTr("Date added"); iconName: "view_time" }
        ListElement { title: qsTr("Title"); iconName: "view_music" }
        ListElement { title: qsTr("Artist"); iconName: "view_artist" }
        ListElement { title: qsTr("Album"); iconName: "view_album" }
    }
    ListModel {
        id: artistmodel
        ListElement { title: qsTr("Date added"); iconName: "view_time" }
        ListElement { title: qsTr("Artist"); iconName: "view_artist" }
    }
    ListModel {
        id: albummodel
        ListElement { title: qsTr("Date added"); iconName: "view_time" }
        ListElement { title: qsTr("Album"); iconName: "view_album" }
    }
    Menu{
        id: menu
        Repeater {
            model: {
                if (sortPageHash === "artist" || sortPageHash === "artistResult") {
                    return artistmodel
                } else if (sortPageHash === "album" || sortPageHash === "albumResult") {
                    return albummodel
                } else {
                    if (sortPageHash === "all" || sortPageHash === "musicResult")
                        listmodel.remove(0)
                    return listmodel
                }
            }
            MenuItem {
                text: title;
                icon.name:iconName;
                autoExclusive: true;
                checked: icon.name == sortBtn.icon.name
                onTriggered: {
                    sortBtn.icon.name = icon.name
                    if (sortPageHash === "album" || sortPageHash === "albumResult") {
                        //console.log("sort " + sortPageHash + "..........................")
                        if (index === 0)
                            Presenter.sortPlaylist(DmGlobal.SortByAddTime, sortPageHash)
                        else if (index === 1)
                            Presenter.sortPlaylist(DmGlobal.SortByAblum, sortPageHash)
                    } else if (sortPageHash === "artist" || sortPageHash === "artistResult") {
                        //console.log("sort " + sortPageHash + "..........................")
                        if (index === 0)
                            Presenter.sortPlaylist(DmGlobal.SortByAddTime, sortPageHash)
                        else if (index === 1)
                            Presenter.sortPlaylist(DmGlobal.SortByArtist, sortPageHash)
                    } else {
                        //console.log("sort ", sortPageHash, "..........................", index)
                        if (sortPageHash === "all" || sortPageHash === "musicResult") {
                            switch (index) {
                            case 0:
                                Presenter.sortPlaylist(DmGlobal.SortByAddTime, sortPageHash)
                                break
                            case 1:
                                Presenter.sortPlaylist(DmGlobal.SortByTitle, sortPageHash)
                                break
                            case 2:
                                Presenter.sortPlaylist(DmGlobal.SortByArtist, sortPageHash)
                                break
                            case 3:
                                Presenter.sortPlaylist(DmGlobal.SortByAblum, sortPageHash)
                                break
                            }
                        } else {
                            switch (index) {
                            case 0:
                                Presenter.sortPlaylist(DmGlobal.SortByCustom, sortPageHash)
                                break
                            case 1:
                                Presenter.sortPlaylist(DmGlobal.SortByAddTime, sortPageHash)
                                break
                            case 2:
                                Presenter.sortPlaylist(DmGlobal.SortByTitle, sortPageHash)
                                break
                            case 3:
                                Presenter.sortPlaylist(DmGlobal.SortByArtist, sortPageHash)
                                break
                            case 4:
                                Presenter.sortPlaylist(DmGlobal.SortByAblum, sortPageHash)
                                break
                            }
                        }
                    }
                }
            }
        }

    }
    onClicked: {
        menu.popup(sortBtn.pressX, sortBtn.pressY);
    }
    onPressAndHold: {
        menu.popup(sortBtn.pressX, sortBtn.pressY);
    }
}
