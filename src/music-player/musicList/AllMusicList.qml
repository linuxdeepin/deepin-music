// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import audio.global 1.0
import "../allItems"
import "../musicmousemenu"

Rectangle {
    id: rootRectangle
    property string listHash: "all" //页面hash值，用于区分自定义歌单、我的喜欢、所有音乐
    property string listTitle: qsTr("All Songs")
    property ListModel mediaModels: AllMusicListModel{dataHash: rootRectangle.listHash}

    objectName: listHash
    color: "transparent"
    height: parent.height/* - 70*/
    Column{
        anchors.fill: rootRectangle
        ToolButtonItem{
            id: toolButtonItem
            width: rootRectangle.width - 40; height: 68;
            title: listTitle
            isPlayAll: rootRectangle.mediaModels.count > 0 ? true : false
            musicinfo: rootRectangle.mediaModels.count === 1 ? qsTr("1 song") : qsTr("%1 songs").arg(rootRectangle.mediaModels.count)
            isDefault: (rootRectangle.mediaModels.count === 0) ? false : true
            pageHash: rootRectangle.listHash
        }
        AllMusicDefaultPage {
            id: defaultPage
            width: toolButtonItem.width; height: rootRectangle.height - toolButtonItem.height
            pageHash: listHash
            visible: (rootRectangle.mediaModels.count === 0) ? true : false
        }
        AllMusicListView {
            id: musicListView
            width: rootRectangle.width
            height: rootRectangle.height - toolButtonItem.height/* - 70*/
            mediaModel: mediaModels
            viewListHash: listHash
        }
    }

    DropArea {
        anchors.fill: parent
        onEntered: {
            drag.accepted = false
            for(var j = 0; j < drag.keys.length; j++) {
                //console.log("onEntered  keys:", drag.keys[j])
                if (drag.keys[j] === "text/uri-list") {
                    drag.accepted = true
                    break
                }
            }
        }

        onDropped: {
            for(var j = 0; j < drop.keys.length; j++) {
                if (drop.keys[j] === "text/uri-list") {
                    var list = []
                    for (var i = 0; i < drop.urls.length; i++)
                        list.push(drop.urls[i])
                    Presenter.importMetas(list, globalVariant.curListPage)
                    break
                }
            }
            drop.accepted = true
        }
    }

    Component.onCompleted: {
        var sortType = Presenter.playlistSortType(listHash)
        //console.log("......................listHash:", listHash, "     sortType:", sortType)
        if (listHash === "all") {
            switch(sortType) {
            case 10:
                toolButtonItem.sortType = 0
                break
            case 11:
                toolButtonItem.sortType = 1
                break
            case 12:
                toolButtonItem.sortType = 2
                break
            case 13:
                toolButtonItem.sortType = 3
                break
            default:
                toolButtonItem.sortType = -1
            }
        } else {
            switch(sortType) {
            case 10:
                toolButtonItem.sortType = 1
                break
            case 11:
                toolButtonItem.sortType = 2
                break
            case 12:
                toolButtonItem.sortType = 3
                break
            case 13:
                toolButtonItem.sortType = 4
                break
            case 14:
                toolButtonItem.sortType = 0
                break
            default:
                toolButtonItem.sortType = -1
            }
        }
    }

    function selectAll() {
        if (musicListView && musicListView.visible) {
            musicListView.selectAll();
        }
    }
}


