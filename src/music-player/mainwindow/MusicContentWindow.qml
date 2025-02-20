// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import org.deepin.dtk 1.0

import "../musicbaseandsonglist"
import "../musicList"
import "../singerlist"
import "../playlist"
import "../albumlist"

Rectangle {
    Component { id: allMusicList ;AllMusicList{} }
    Component { id: albumView ;AlbumView{} }
    Component { id: artistView ;ArtistView{} }
    Component { id: favouriteSongsList ;AllMusicList{listHash: "fav"; listTitle: qsTr("My Favorites")} }

    id: contentWindow
    color: "transparent"
    signal clickPlayAllBtn()

    MusicBaselistview {
        id: musicBaselist
        width: 220;
        height: parent.height/* - 70*/
        anchors {
            left: parent.left
            top: parent.top
            topMargin: 50
        }
    }
    StackView {
        id: listViewStackView
        width: parent.width - musicBaselist.width
        height: parent.height - 70
        anchors.left: musicBaselist.right
        anchors.top: parent.top
        anchors.topMargin: 50
        clip: true
        initialItem: AllMusicList{}
    }

    function onSearchResultItemChanged(text, type) {
        globalVariant.curListPage = "search"
        var item = listViewStackView.find(function(item, index) { return item.objectName === "search" })
        if(item !== null){
            item.updateSearchResultInfo(text, type)
            listViewStackView.pop(item);
            return;
        }

        listViewStackView.push(Qt.resolvedUrl("SearchResultWindow.qml"), {pattern:text, curIndex: type});
    }

    function selectAll() {
        var item = listViewStackView.currentItem;
        if (item && item.objectName !== "artist" && item.objectName !== "album") {
            item.selectAll();
        }
    }

    Connections {
        target: musicBaselist;
        onMusicBaselistChanged: {
            globalVariant.globalSwitchButtonStatus = 0; //初始化切换按钮状态
            globalVariant.curListPage = type
            var item = listViewStackView.find(function(item, index) { return item.objectName === type })
            if(item !== null){
                if(item.objectName === "artist" || item.objectName === "album"){
                    item.returnUpperlevelView(); //回到上一级页面
                }
                listViewStackView.pop(item);
                return;
            }
            if(type == "all"){        
                listViewStackView.push(allMusicList);
            }else if (type == "artist"){
               listViewStackView.push(artistView);
            }else if (type == "album"){
                listViewStackView.push(albumView);
            }else if(type == "fav"){
               listViewStackView.push(favouriteSongsList);
            }else{
                listViewStackView.push(Qt.resolvedUrl("../musicList/AllMusicList.qml"), {listHash:type, listTitle:displayName});
            }
        }
    }
    Connections {
        target: globalVariant
        onUpdateCurrentPlaylistTitleName: {
            var item = listViewStackView.find(function(item, index) { return item.objectName === curHash }); //重命名后，更新列表标题名
            if(item !== null){
                item.listTitle = name;
                return;
            }
        }
    }
}
