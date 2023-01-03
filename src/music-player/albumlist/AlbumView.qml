// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import "../allItems"
import "../musicsublist"

Rectangle {
    property int switchType: globalVariant.globalSwitchButtonStatus;
    property ListModel albumModels: AlbumModel{}
    //采用名字作为索引，确保qml能够识别，
    // property var artistData: artistModels.get(0)qml可能无法识别
    property string albumName:  albumModels.get(0).name
    signal itemDoubleClicked(var albumData)

    id: contenWindow
    color: "transparent"
    objectName: "album"

    Component {
        id: albumSublistView
        AlbumSublistView {
            albumData: {
                for(var i = 0; i < albumModels.count; i++){
                    if(albumName === albumModels.get(i).name){
                        return albumModels.get(i);
                    }
                }
            }
        }
    }

    StackView {
        id: stackView
        width: contenWindow.width; height: contenWindow.height
        anchors.left: contenWindow.left;
        clip: true
        initialItem: Rectangle {
            id: albumView
            color: "transparent"
            ToolButtonItem{
                id: toolButtonItem
                width: contenWindow.width - 40; height: 68;
                title: qsTr("Albums")
                musicinfo: qsTr("%1 albums - %2 songs").arg(contenWindow.albumModels.count).arg(contenWindow.albumModels.musicCount)
                gridAndlistViewModel: (contenWindow.albumModels.count === 0) ? false : true
                isDefault: (contenWindow.albumModels.count === 0) ? false : true
                pageHash: "album"
            }
            AlbumDefaultPage {
                id: defaultPage
                width: toolButtonItem.width; height: contenWindow.height - toolButtonItem.height
                anchors.left: toolButtonItem.left
                anchors.top: toolButtonItem.bottom;
                visible: (contenWindow.albumModels.count === 0) ? true : false
            }

            AlbumListView {
                id: listview
                width: parent.width
                height: stackView.height - toolButtonItem.height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: toolButtonItem.bottom;
                visible: false;
                albumModels: contenWindow.albumModels
                onItemDoubleClicked:{
                    contenWindow.itemDoubleClicked(albumData);
                }
            }

            AlbumGridView {
                id: gridview
                width: parent.width; height: parent.height - toolButtonItem.height
                anchors.left: parent.left; /*anchors.leftMargin: 10*/
                anchors.top: toolButtonItem.bottom;
                visible: (contenWindow.albumModels.count === 0) ? false : true;
                albumModel: contenWindow.albumModels
                onItemDoubleClicked:{
                    contenWindow.itemDoubleClicked(albumData);
                }
            }

            DropArea {
                anchors.fill: parent
                onDropped: {
                    var list = []
                    for (var i = 0; i < drop.urls.length; i++)
                        list.push(drop.urls[i])
                    Presenter.importMetas(list, globalVariant.curListPage)
                }
            }

            Connections {
                target: toolButtonItem
                onViewChanged:{
                    if(type === 0){
                        gridview.visible = true;
                        listview.visible = false
                    }else{
                        listview.visible = true;
                        gridview.visible = false;
                    }
                }
            }
        }
    }
    onItemDoubleClicked: {
        contenWindow.albumName = albumData.name
        globalVariant.globalSwitchButtonStatus = 2; //使能上一页按钮，失能下一页按钮
    }
    onSwitchTypeChanged: {
        if(switchType === 1){
            stackView.pop(stackView.initialItem); //回到上一级页面
            globalVariant.globalSwitchButtonStatus = 1; //使能下一页按钮，失能上一页按钮
        }else if(switchType === 2){
            var item = stackView.find(function(item, index) { return item.objectName === "albumSublist"})
            if(item !== null){
                stackView.pop(item);
                return;
            }
            stackView.push(albumSublistView); //切换到下一级页面
        }
    }

    function returnUpperlevelView(){
        globalVariant.globalSwitchButtonStatus = 0; //初始化切换按钮状态
        stackView.pop(stackView.initialItem); //回到上一级页面
    }

    Connections {
        target: globalVariant
        onReturnUpperlevelView: {contenWindow.returnUpperlevelView();}
    }
    Connections {
        target: albumModels
        onMetaCodecChanged: {
            contenWindow.albumName = name;
        }
    }

    Component.onCompleted: {
        var sortType = Presenter.playlistSortType("album")
        if (sortType === 10)
            toolButtonItem.sortType = 0
        else if (sortType === 13)
            toolButtonItem.sortType = 1
    }
}
