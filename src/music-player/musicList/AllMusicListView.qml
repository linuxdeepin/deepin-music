// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import audio.global 1.0
import "../allItems"
import "../musicmousemenu"
import "../dialogs"

Rectangle {
    property ListModel mediaModel
    property string viewListHash: ""
    property double scalingRatio: 168 / 810  //计算宽度占比
    Loader { id: importMenuLoader }
    Loader { id: moreMenuLoader }
    Loader { id: selectMenuLoader }

    id: musicListView
    color: "transparent"
    //标题栏
    Row {
        id: headerView
        width: musicListView.width - 40; height: 36
        leftPadding: 20
        Rectangle {
            width: 56; height: 36
            color: "transparent"
        }
        Label {
            width: parent.width - 2 * parent.width * scalingRatio - 158
            height: 36
            leftPadding: 10
            text: qsTr("Title")
            verticalAlignment: Qt.AlignVCenter
        }
        Label {
            width: parent.width * scalingRatio; height: 36
            leftPadding: 10
            text: qsTr("Artist")
            verticalAlignment: Qt.AlignVCenter
        }
        Label {
            width: parent.width * scalingRatio; height: 36
            text: qsTr("Album")
            verticalAlignment: Qt.AlignVCenter
        }
        Label {
            width: 102; height: 36
            text: qsTr("Duration")
            verticalAlignment: Qt.AlignVCenter
        }
    }

    function selectAll() {
        if (listview && listview.visible && mediaModel.count > 0) {
            listview.selectAll();
        }
    }

    ListView {
        property var delegateModelGroup: new Array
        property var dragGroup: new Array
        property int lastIndex: 0
        property int dragToIndex: 0
        property MusicInfoDialog infoDialog: MusicInfoDialog{musicData: listview.model.get(0)}

        id: listview
        width: parent.width
        height: parent.height - 38
        anchors.top: headerView.bottom;
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}
        model: mediaModel
        clip: true
        focus: true
        visible: (mediaModel.count === 0) ? false : true
        delegate: AllMusicListDelegate{
            id: itemD
            autoExclusive: false
            width: listview.width - 40
            height: 56
            backgroundVisible: true
            normalBackgroundVisible: index % 2 === 0
            delegateListHash: viewListHash
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
                    listview.checkMulti(mediaModels.count -1);
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
                    infoDialog.musicData = mediaModel.get(lastIndex);
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

        DeleteSonglistDialog {id: removeSong; listHash: viewListHash}

        DropArea {
            property int lastDragIndex: 0
            property int toIndex: 0
            property int hoverIndex: 0
            property bool dragForSort: false

            id: dropArea
            anchors.fill: parent

            onEntered: {
                for(var j = 0; j < drag.keys.length; j++) {
                    if (drag.keys[j] === "music-list/index-list" && Presenter.playlistSortType(viewListHash) === DmGlobal.SortByCustom) {
                        dragForSort = true
                        break
                    }
                }

                if (Presenter.playlistSortType(viewListHash) === DmGlobal.SortByCustom
                        && viewListHash !== "cdarole")
                    drag.accepted = true
                else
                    drag.accepted = false
            }
            onPositionChanged: {
                updateHoverIndex()

                if (drag.y < 20 && !listview.atYBeginning) {
                    scrollUpTimer.start()
                } else {
                    scrollUpTimer.stop()
                }

                if (drag.y > listview.height - 20 && !listview.atYEnd) {
                    scrollDownTimer.start()
                } else {
                    scrollDownTimer.stop()
                }
            }
            onDropped: {
                updateHoverIndex()
                if (dragForSort) {
                    scrollDownTimer.stop()
                    scrollUpTimer.stop()

                    var hashList = []
                    for (var i = 0; i < listview.delegateModelGroup.length; i++){
                        hashList.push(mediaModel.get(listview.delegateModelGroup[i]).hash);
                    }
                    if (toIndex + 1 >= mediaModel.count) {
                        Presenter.moveMetasPlayList(hashList, viewListHash, "")
                    } else {
                        Presenter.moveMetasPlayList(hashList, viewListHash, mediaModel.get(toIndex + 1).hash)
                    }

                    listview.delegateModelGroup.sort()

                    var temp = 0
                    for (var i = 0; i < listview.delegateModelGroup.length; i++) {
                        if (listview.delegateModelGroup[i] <= toIndex) {
                            mediaModel.move(listview.delegateModelGroup[i] - temp, toIndex, 1) //从前向后移动，每移动一个后剩余的项下标就减1
                            temp++
                        } else {
                            toIndex++
                            mediaModel.move(listview.delegateModelGroup[i], toIndex, 1) //从后向前移动，每移动一个后目标索引就会加1
                        }
                        mediaModel.setProperty(toIndex, "inMulitSelect", false);
                        mediaModel.setProperty(toIndex - 1, "dragFlag", false);
                    }
                    listview.removeModelGroup()
                }
                dragForSort = false
            }
            onExited: {
                mediaModel.setProperty(lastDragIndex, "dragFlag", false)
                scrollDownTimer.stop()
                scrollUpTimer.stop()
                dragForSort = false
            }

            function updateHoverIndex() {
                hoverIndex = listview.indexAt(drag.x, drag.y + listview.contentY)

                if (drag.y + listview.contentY < hoverIndex * 56 + 56 / 2)
                    hoverIndex--
                if (hoverIndex < 0)
                    hoverIndex = -1

                if (hoverIndex !== lastDragIndex) {
                    if (hoverIndex >= 0)
                        mediaModel.setProperty(hoverIndex, "dragFlag", true)
                    if (lastDragIndex >= 0)
                        mediaModel.setProperty(lastDragIndex, "dragFlag", false)
                }

                toIndex = hoverIndex
                lastDragIndex = hoverIndex
                listview.dragToIndex = hoverIndex
            }
        }

        Timer {
            id: scrollDownTimer
            interval: 40
            repeat: true
            running: false

            onTriggered: {
                if(!listview.atYEnd)
                    listview.contentY += 10
            }
        }
        Timer {
            id: scrollUpTimer
            interval: 40
            repeat: true
            running: false

            onTriggered: {
                if(!listview.atYBeginning)
                    listview.contentY -= 10
            }
        }

        function removeModelGroup(){
            for(var i = delegateModelGroup.length - 1; i >= 0; i--){
                mediaModel.setProperty(delegateModelGroup[i], "inMulitSelect", false);
                delegateModelGroup.pop(i);
                dragGroup.pop(i);
            }
        }
        function keysShiftModifier(){
            if(listview.lastIndex <= 0)
                listview.lastIndex = 0;
            if(listview.lastIndex >= mediaModel.count)
                listview.lastIndex = mediaModel.count -1;
            if(isShiftModifier){
                var inMulitSelect = mediaModel.get(listview.lastIndex).inMulitSelect;
                mediaModel.setProperty(listview.lastIndex, "inMulitSelect", (!inMulitSelect));
                if((!inMulitSelect) === false){
                    listview.delegateModelGroup.pop();
                    listview.dragGroup.pop();
                }else{
                    listview.delegateModelGroup.push(listview.lastIndex);
                    listview.dragGroup.push(mediaModel.get(listview.lastIndex).coverUrl)
                }
            }else{
                listview.checkOne(listview.lastIndex);
                listview.currentIndex = listview.lastIndex;
            }
        }
        function checkOne(idx){
            //console.log("checkOne......", idx)
            listview.removeModelGroup();
            mediaModel.setProperty(idx, "inMulitSelect", true);
            listview.delegateModelGroup.push(idx);
            listview.dragGroup.push(mediaModel.get(idx).coverUrl)
            listview.lastIndex = idx;
        }
        function checkMulti(idx){
            //console.log("checkMulti......", idx)
            listview.removeModelGroup();
            var beging = (listview.lastIndex >= idx) ? idx : listview.lastIndex
            var end = (listview.lastIndex >= idx) ? listview.lastIndex: idx
            for(var i = beging; i <= end; i++){
                mediaModel.setProperty(i, "inMulitSelect", true);
                listview.delegateModelGroup.push(i);
                listview.dragGroup.push(mediaModel.get(i).coverUrl)
            }
            listview.lastIndex = idx;
        }
        function getSelectGroupHashList(){
            var hashList = [];
            for(var j = 0; j < listview.delegateModelGroup.length; j++){
                if(mediaModel.get(listview.delegateModelGroup[j]).inMulitSelect){
                    var tmpHash  = mediaModel.get(listview.delegateModelGroup[j]).hash;
                    hashList.push(tmpHash);
                    removeSong.musicTitle = mediaModel.get(listview.delegateModelGroup[j]).title;
                }
            }
            return hashList;
        }
        function delectSelectMusices(){
            var tmpSelect = getSelectGroupHashList();
            removeSong.deleteHashList = tmpSelect;
            removeSong.show();
        }

        onVisibleChanged: {
            //console.log("onVisibleChanged...................", viewListHash, "    ", visible)
            if (!visible) {
                for (var i = 0; i < delegateModelGroup.length; i++) {
                    mediaModel.setProperty(delegateModelGroup[i], "inMulitSelect", false);
                }
                delegateModelGroup = []
                dragGroup = []
                globalVariant.currentSelectMediaMeta = null
            }
        }
        onContentYChanged: {
//            console.log("onContentYChanged..................")
            if (dropArea.dragForSort)
                dropArea.updateHoverIndex()
        }

        Connections {
            target: globalVariant
            onClearSelectGroup: {listview.removeModelGroup()}
        }

        function selectAll() {
            listview.removeModelGroup();
            for(var i = 0; i < mediaModel.count; i++) {
                mediaModel.setProperty(i, "inMulitSelect", true);
                listview.delegateModelGroup.push(i);
                listview.dragGroup.push(mediaModel.get(i).coverUrl);
            }
            listview.lastIndex = mediaModel.count - 1;
        }
    }
}
