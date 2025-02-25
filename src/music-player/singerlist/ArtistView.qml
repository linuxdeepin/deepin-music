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
import "../musicsublist"

Rectangle {
    id: contenWindow
    color: "transparent"
    objectName: "artist"
    property ListModel artistModels: ArtistModel{}
    signal itemDoubleClicked(var artistData)
    property int switchType: globalVariant.globalSwitchButtonStatus;
    property point currentItemPos: [0, 0]
    property Menu artistMoreMenu: ArtistMoreMenu{}
    //采用名字作为索引，确保qml能够识别，
    // property var artistData: artistModels.get(0) qml可能无法识别
    property string artistDataName: artistModels.get(0).name
    Component {
        id: artistSublistView
        ArtistSublistView {
            scalePoint: currentItemPos
            artistData: {
                for(var i = 0; i < artistModels.count; i++){
                    if(artistDataName === artistModels.get(i).name){
                        return artistModels.get(i);
                    }
                }
            }
        }
    }

    StackView {
        id: myStackView
        anchors.fill: contenWindow
        width: contenWindow.width; height: contenWindow.height
        clip: true
        initialItem: Rectangle {
            id: rootRectangle
            color: Qt.rgba(0, 0, 0, 0)
            ToolButtonItem{
                id: toolButtonItem
                isPlayAll: contenWindow.artistModels.count > 0 ? true : false
                width: rootRectangle.width - 40; height: 68;
                anchors.left: rootRectangle.left; anchors.leftMargin: 20
                title: qsTr("Artists")
                musicinfo: qsTr("%1 artists - %2 songs").arg(contenWindow.artistModels.count).arg(contenWindow.artistModels.musicCount)
                isDefault: (contenWindow.artistModels.count === 0) ? false : true
                pageHash: "artist"
            }
            ArtistDefaultPage {
                id: defaultPage
                width: toolButtonItem.width; height: rootRectangle.height - toolButtonItem.height
                anchors.left: toolButtonItem.left; /*anchors.leftMargin: 20*/
                anchors.top: toolButtonItem.bottom;
                visible: (contenWindow.artistModels.count === 0) ? true : false
            }
            Rectangle {
                id: viewParent;
                width: parent.width;
                height: rootRectangle.height - toolButtonItem.height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: toolButtonItem.bottom
                color: Qt.rgba(0, 0, 0, 0)
                clip: true
                visible: (contenWindow.artistModels.count === 0) ? false : true;
                GridView {
                    id: singergridview
                    width: Math.floor((viewParent.width - 20) / 208) * 208;
                    height: rootRectangle.height - toolButtonItem.height - 34
                    anchors.horizontalCenter: viewParent.horizontalCenter
                    anchors.top: viewParent.top
                    anchors.topMargin: 34
                    anchors.left: viewParent.left
                    anchors.leftMargin: (viewParent.width - width) / 2
                    cellWidth: 208; cellHeight: 230
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar {}
                    model: artistModels
                    delegate: ArtistGridDelegate{
                        id: musicSingerGridItem
                        onItemDoubleClicked: {
                            currentItemPos.x = musicSingerGridItem.x + musicSingerGridItem.width / 2
                            currentItemPos.y = musicSingerGridItem.y + musicSingerGridItem.height / 2 - 20
                            contenWindow.itemDoubleClicked(artistData);
                        }
                    }
                    onWidthChanged: {
                        var w =  Math.floor(viewParent.width / 208) * 208;
                        var m = (viewParent.width - w) / 2;
                        anchors.leftMargin = m;
//                        anchors.rightMargin = m;
                    }
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
        }

        popEnter: Transition {
            // slide_in_left
            NumberAnimation { property: "xScale"; from: 0; to: 1; duration: 300; easing.type: Easing.InOutQuad }
            NumberAnimation { property: "yScale"; from: 0; to: 1; duration: 300; easing.type: Easing.InOutQuad }
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 300; easing.type: Easing.InOutQuad }
        }

        popExit: Transition {
            // slide_out_right
            NumberAnimation { property: "xScale"; from: 1; to: 0; duration: 300; easing.type: Easing.InOutQuad }
            NumberAnimation { property: "yScale"; from: 1; to: 0; duration: 300; easing.type: Easing.InOutQuad }
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 200; easing.type: Easing.Easing.OutExpo }
        }

        pushEnter: Transition {
            // slide_in_right
            NumberAnimation { property: "xScale"; from: 0; to: 1; duration: 300; easing.type: Easing.InOutQuad }
            NumberAnimation { property: "yScale"; from: 0; to: 1; duration: 300; easing.type: Easing.InOutQuad }
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 300; easing.type: Easing.InOutQuad }
        }

        pushExit: Transition {
            // slide_out_left
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 200; easing.type: Easing.Easing.OutExpo }
        }
    }
    onItemDoubleClicked: {
        contenWindow.artistDataName = artistData.name;
        globalVariant.globalSwitchButtonStatus = 2; //使能上一页按钮，失能下一页按钮
    }
    onSwitchTypeChanged: {
        if(switchType === 1){
            myStackView.pop(myStackView.initialItem); //回到上一级页面
        }else if(switchType === 2){
            var item = myStackView.find(function(item, index) { return item.objectName === "artistSublist"})
            if(item !== null){
                myStackView.pop(item);
                return;
            }
            myStackView.push(artistSublistView); //切换到下一级页面
        }
    }
    function returnUpperlevelView(){
        globalVariant.globalSwitchButtonStatus = 0; //初始化切换按钮状态
        myStackView.pop(myStackView.initialItem); //回到上一级页面
    }
    Connections {
        target: artistMoreMenu
        onViewArtistDatails: { contenWindow.itemDoubleClicked(artistData);}
    }
    Connections {
        target: globalVariant
        onReturnUpperlevelView: {contenWindow.returnUpperlevelView();}
    }
    Connections {
        target: artistModels
        onMetaCodecChanged: {
            contenWindow.artistDataName = name;
        }
    }

    Component.onCompleted: {
        var sortType = Presenter.playlistSortType("artist")
        //console.log("ArtistView onCompleted:      sortType", sortType)
        if (sortType === 10)
            toolButtonItem.sortType = 0
        else if (sortType === 12)
            toolButtonItem.sortType = 1
    }
}
