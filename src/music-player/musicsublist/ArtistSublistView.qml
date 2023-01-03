// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import "../allItems"
import "../musicmousemenu"
import "../dialogs"
Rectangle {
    property double scalingRatio: 168 / 810   //计算宽度占比
    property var artistData/*: globalVariant.globalSublistData*/ //ArtistInfo 对象
    property int m_mouseY
    property bool m_isPressed
    property ListModel mediaListModels: MusicSublistModel{ meidaDataMap: artistData.musicinfos} //从ArtistInfo中解析musicinfos
    property Menu importMenu: ImportMenu{}
    property Menu musicMoreMenu: MusicMoreMenu{pageHash: "artist"}
    property Menu selectMenu: MulitSelectMenu{pageHash: "artist"}

    id: rootrectangle
    objectName: "artistSublist"
    color: "transparent"
    MusicSublistTitle {
        id: musicSublistTitle
        anchors.left: rootrectangle.left; anchors.top: rootrectangle.top
        titleWidth: rootrectangle.width; titleHeight: 244
        currentData: rootrectangle.artistData  //ArtistInfo 对象
        pageHash: "artist"
    }

    Row {
        id: headerView
        anchors.top: musicSublistTitle.bottom
        anchors.left: musicSublistTitle.left; anchors.leftMargin: 20
        width: musicSublistTitle.width - 40; height: 36
        Rectangle {
            width: 26; height: 36
            color: "transparent"
        }
        Label {
            width: headerView.width - 368; height: 36
            leftPadding: 10
            text: qsTr("Title")
            verticalAlignment: Qt.AlignVCenter
        }
        Label {
            width: 200; height: 36
            leftPadding: 10
            text: qsTr("Album")
            verticalAlignment: Qt.AlignVCenter
        }
        Label {
            width: 142; height: 36
            text: qsTr("Duration")
            verticalAlignment: Qt.AlignVCenter
        }
    }

    ListView{
        id: listview
        property var delegateModelGroup: new Array
        property int lastIndex: 0
        width: rootrectangle.width
        height: rootrectangle.height - musicSublistTitle.height - 36
        anchors.left: musicSublistTitle.left/*; anchors.leftMargin: 20*/
        anchors.top: musicSublistTitle.bottom; anchors.topMargin: 36
        ScrollBar.vertical: ScrollBar {
            id:artistSublistScrollBar
            stepSize:  mediaListModels.count > 1 ? 1 / mediaListModels.count : 0.1
        }

        model: mediaListModels
        clip: true
        focus: true
        property MusicInfoDialog infoDialog: MusicInfoDialog{musicData: listview.model.get(0)}
        delegate: ArtistSublistDelegate{
            width: listview.width - 40
            height: 56
            anchors.horizontalCenter: parent.horizontalCenter
            backgroundVisible: index % 2 === 0
            autoExclusive: false
            checked: mediaListModels.get(index).inMulitSelect
        }

        MouseArea{
            anchors.fill: parent
            acceptedButtons:Qt.NoButton
            onWheel: {
                if(wheel.angleDelta.y>1){
                    artistSublistScrollBar.decrease()
                } else{
                    artistSublistScrollBar.increase()
                }
                if(wheel.angleDelta.y < 1 && artistSublistScrollBar.position > 0){
                    musicSublistTitle.titleHeight = 80;
                    musicSublistTitle.suspensionTitle(true);
                }else {
                    musicSublistTitle.titleHeight = 244;
                    musicSublistTitle.suspensionTitle(false);
                }
            }
        }

        function removeModelGroup(){
            for(var i = delegateModelGroup.length - 1; i >= 0; i--){
                mediaListModels.setProperty(delegateModelGroup[i], "inMulitSelect", false);
                delegateModelGroup.pop(i);
            }
        }
        function keysShiftModifier(){
            if(listview.lastIndex <= 0) listview.lastIndex = 0;
            if(listview.lastIndex >= mediaListModels.count) listview.lastIndex = mediaListModels.count -1;
            if(isShiftModifier){
                var inMulitSelect = mediaListModels.get(listview.lastIndex).inMulitSelect;
                mediaListModels.setProperty(listview.lastIndex, "inMulitSelect", (!inMulitSelect));
                if((!inMulitSelect) === false){
                    listview.delegateModelGroup.pop();
                }else{
                    listview.delegateModelGroup.push(listview.lastIndex);
                }
            }else{
                listview.checkOne(listview.lastIndex);
                listview.currentIndex = listview.lastIndex;
            }
        }

        function checkOne(idx){
            listview.removeModelGroup();
            mediaListModels.setProperty(idx, "inMulitSelect", true);
            listview.delegateModelGroup.push(idx);
            listview.lastIndex = idx;
        }

        function checkMulti(idx){
            listview.removeModelGroup();
            var beging = (listview.lastIndex >= idx) ? idx : listview.lastIndex
            var end = (listview.lastIndex >= idx) ? listview.lastIndex: idx
            for(var i = beging; i <= end; i++){
                mediaListModels.setProperty(i, "inMulitSelect", true);
                listview.delegateModelGroup.push(i);
            }
            listview.lastIndex = idx;
        }

        function getSelectGroupHashList(){
            var hashList = [];
            for(var j = 0; j < listview.delegateModelGroup.length; j++){
                if(mediaListModels.get(listview.delegateModelGroup[j]).inMulitSelect){
                    var tmpHash  = mediaListModels.get(listview.delegateModelGroup[j]).hash;
                    hashList.push(tmpHash);
                    removeSong.musicTitle = mediaListModels.get(listview.delegateModelGroup[j]).title;
                }
            }
            return hashList;
        }
        DeleteSonglistDialog {id: removeSong; listHash: "album"}
        function delectSelectMusices(){
            var tmpSelect = getSelectGroupHashList();
            removeSong.deleteHashList = tmpSelect;
            removeSong.show();
        }
        property bool isShiftModifier: false;
        property int keyChanged: 0; //如果方向改变,该值也会改变
        Keys.onPressed: {
            switch (event.key){
            case Qt.Key_Up:
                listview.lastIndex--;
                if(isShiftModifier && keyChanged === 2) listview.lastIndex++
                keyChanged = 1;
                listview.keysShiftModifier();
                break;
            case Qt.Key_Down:
                listview.lastIndex++;
                if(isShiftModifier && keyChanged === 1) listview.lastIndex--
                keyChanged = 2;
                listview.keysShiftModifier();
                break;
            case Qt.Key_A:
                if (event.modifiers & Qt.ControlModifier) {
                    listview.lastIndex = 0;
                    listview.checkMulti(mediaListModels.count -1);
                }
                break;
            case Qt.Key_Shift:
                listview.isShiftModifier = true;
                break;
            case Qt.Key_Delete:
                listview.delectSelectMusices();
                break;
            case Qt.Key_L:
                if (event.modifiers & Qt.ControlModifier) {
                    infoDialog.musicData = mediaListModels.get(lastIndex);
                    infoDialog.show();
                }
                break;
            default:
                break;
            }
            event.accepted = true;
        }
        Keys.onReleased: {
            if(event.key === Qt.Key_Shift){
                listview.isShiftModifier = false;
            }
        }
        Connections {
            target: globalVariant
            onClearSelectGroup: {listview.removeModelGroup()}
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
