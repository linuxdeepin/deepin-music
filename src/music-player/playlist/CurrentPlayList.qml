// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS

import audio.global 1.0
import "../musicmousemenu"
import "../dialogs"

CurrentFloatingPanel {
    property int  headerHeight: 73
    property ListModel listmodel: PlayListModel{}
    property Menu playlistMoreMenu: MusicMoreMenu{pageHash:"play"}
    property Menu playlistAddMenu: ImportMenu{pageHash:"play"}
    property Menu selectMenu: MulitSelectMenu{pageHash: "play"}
    property MusicInfoDialog infoDialog: MusicInfoDialog{musicData: playlistView.model.get(0)}
    signal playlistHided()
    signal playlistEmpty()

    id: playlistRoot
    visible: isPlaylistShow
    width: 320
    height: parent.height - 90
    radius: 8
    // background: InWindowBlur {
    //     implicitWidth: DS.Style.floatingMessage.panel.width
    //     implicitHeight: DS.Style.floatingMessage.panel.height
    //     anchors.fill: parent
    //     radius: 32
    //     offscreen: true

    //     ItemViewport {
    //         anchors.fill: parent
    //         fixed: true
    //         sourceItem: parent
    //         radius: playlistRoot.radius
    //         hideSource: false
    //     }

    //     BoxShadow {
    //         anchors.fill: backgroundRect
    //         shadowOffsetX: 0
    //         shadowOffsetY: 4
    //         shadowColor: playlistRoot.D.ColorSelector.borderColor
    //         shadowBlur: 20
    //         cornerRadius: backgroundRect.radius
    //         spread: 0
    //         hollow: true
    //     }
    //     Rectangle {
    //         id: backgroundRect
    //         anchors.fill: parent
    //         radius: playlistRoot.radius
    //         color: playlistRoot.D.ColorSelector.borderColor
    //         border {
    //             //color: playlistRoot.D.ColorSelector.borderColor
    //             width: /*DS.Style.control.borderWidth*/0
    //         }
    //     }
    // }

    MouseArea {
        anchors.fill: parent

        onWheel: {
            wheel.accepted = true
        }
    }

    Column {
        width: parent.width
        height: parent.height
        Rectangle {
            id: headerArea
            width: parent.width
            height: headerHeight
            color: "#00000000"

            Column {
                spacing: 6
                topPadding: 10
                leftPadding: 10
                width: parent.width - 20
                height: parent.height

                Rectangle {
                    width: parent.width
                    height: 26
                    color: "#00000000"
                    Text {
                        id: playlistText
                        text: qsTr("Play queue")
                        font: DTK.fontManager.t5
                        color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(247, 247, 247, 0.9) : Qt.rgba(0, 0, 0, 0.9)
                    }
                }
                Rectangle {
                    width: parent.width
                    height: 17
                    color: "#00000000"

                    Rectangle {
                        width: childrenRect.width
                        height: parent.height
                        anchors.left: parent.left
                        color: "#00000000"
                        Text {
                            id: songsCountText
                            text: listmodel.count === 1 ? qsTr("1 song") : qsTr("%1 songs").arg(listmodel.count)
                            color: "#7c7c7c"
                            font: DTK.fontManager.t7
                        }
                    }

                    ToolButton {
                        id: deleteBtn
                        height: 20
                        anchors.right: parent.right
                        anchors.rightMargin: 2
                        icon.name: "playlist_delete"
                        icon.width: 20
                        icon.height: 20
                        text: qsTr("Empty")
                        font: DTK.fontManager.t9
                        display: AbstractButton.TextBesideIcon
                        spacing: 1
                        padding: 0
                        textColor: Palette {
                            normal: palette.text
                            normalDark: palette.text
                            hovered: palette.highlight
                        }

                        onClicked: {
                            playlistEmpty()
                            listmodel.clear()
                            Presenter.clearPlayList("play")
                        }
                    }
                }
            }
        }
        ListView {
            property var delegateModelGroup: new Array
            property var dragGroup: new Array
            property int lastIndex: 0
            property int dragToIndex: 0

            id: playlistView
            width: parent.width
            height: parent.height - headerHeight
            anchors.left: parent.left
            ScrollBar.vertical: ScrollBar {}
            boundsBehavior: Flickable.StopAtBounds
            clip: true
            focus: true
            model: listmodel
            delegate: CurrentPlayListDelegate {
                id: playlistDelegate
                width: 300
                height: 56
                anchors.left: parent.left
                anchors.leftMargin: 10
                backgroundVisible: true
                normalBackgroundVisible: index % 2 === 0
                autoExclusive: false
            }

            DropArea {
                property int lastDragIndex: 0
                property int toIndex: 0
                property int hoverIndex: 0
                property bool dragForSort: false

                id: dropArea
                anchors.fill: parent

                onEntered: {
                    drag.accepted = true
                    for(var j = 0; j < drag.keys.length; j++) {
                        if (drag.keys[j] === "music-list/index-list") {
                            dragForSort = true
                            break
                        }
                    }
                }
                onPositionChanged: {
                    updateHoverIndex()

                    if (drag.y < 20 && !playlistView.atYBeginning) {
                        scrollUpTimer.start()
                    } else {
                        scrollUpTimer.stop()
                    }

                    if (drag.y > playlistView.height - 20 && !playlistView.atYEnd) {
                        scrollDownTimer.start()
                    } else {
                        scrollDownTimer.stop()
                    }
                }
                onDropped: {
                    if (dragForSort) {
                        scrollDownTimer.stop()
                        scrollUpTimer.stop()

                        var hashList = []
                        for (var i = 0; i < playlistView.delegateModelGroup.length; i++){
                            hashList.push(listmodel.get(playlistView.delegateModelGroup[i]).hash);
                        }
                        if (toIndex + 1 >= listmodel.count)
                            Presenter.moveMetasPlayList(hashList, "play", "")
                        else
                            Presenter.moveMetasPlayList(hashList, "play", listmodel.get(toIndex + 1).hash)

                        playlistView.delegateModelGroup.sort()

                        var temp = 0
                        for (var i = 0; i < playlistView.delegateModelGroup.length; i++){

                            if (playlistView.delegateModelGroup[i] <= toIndex) {
                                listmodel.move(playlistView.delegateModelGroup[i] - temp, toIndex, 1) //从前向后移动，每移动一个后剩余的项下标就减1
                                temp++
                            } else {
                                toIndex++
                                listmodel.move(playlistView.delegateModelGroup[i], toIndex, 1) //从后向前移动，每移动一个后目标索引就会加1
                            }
                            listmodel.setProperty(toIndex, "inMulitSelect", false);
                            listmodel.setProperty(toIndex - 1, "dragFlag", false);
                        }
                        playlistView.removeModelGroup()
                    }
                    dragForSort = false
                }
                onExited: {
                    if (lastDragIndex >= 0)
                        listmodel.setProperty(lastDragIndex, "dragFlag", false)
                    scrollDownTimer.stop()
                    scrollUpTimer.stop()
                    dragForSort = false
                    drag.accepted = false
                }

                function updateHoverIndex() {
                    hoverIndex = playlistView.indexAt(drag.x, drag.y + playlistView.contentY)

                    if (drag.y + playlistView.contentY < hoverIndex * 56 + 56 / 2)
                        hoverIndex--
                    if (hoverIndex < 0)
                        hoverIndex = -1

                    if (hoverIndex !== lastDragIndex) {
                        if (hoverIndex >= 0)
                            listmodel.setProperty(hoverIndex, "dragFlag", true)
                        if (lastDragIndex >= 0)
                            listmodel.setProperty(lastDragIndex, "dragFlag", false)
                    }

                    toIndex = hoverIndex
                    lastDragIndex = hoverIndex
                    playlistView.dragToIndex = hoverIndex
                }
            }

            DeleteSonglistDialog {id: removeSong; listHash: "play"}

            Timer {
                id: scrollDownTimer
                interval: 40
                repeat: true
                running: false

                onTriggered: {
                    if(!playlistView.atYEnd) {
                        playlistView.contentY += 10
                    }
                }
            }
            Timer {
                id: scrollUpTimer
                interval: 40
                repeat: true
                running: false

                onTriggered: {
                    if(!playlistView.atYBeginning) {
                        playlistView.contentY -= 10
                    }
                }
            }

            onContentYChanged: {
                if (dropArea.dragForSort)
                    dropArea.updateHoverIndex()
            }
            function removeModelGroup(){
                for(var i = delegateModelGroup.length - 1; i >= 0; i--){
                    listmodel.setProperty(delegateModelGroup[i], "inMulitSelect", false);
                    delegateModelGroup.pop(i);
                    dragGroup.pop(i)
                }
            }

            function keysShiftModifier(){
                if(playlistView.lastIndex <= 0) playlistView.lastIndex = 0;
                if(playlistView.lastIndex >= listmodel.count) playlistView.lastIndex = listmodel.count -1;
                if(isShiftModifier){
                    var inMulitSelect = listmodel.get(playlistView.lastIndex).inMulitSelect;
                    listmodel.setProperty(playlistView.lastIndex, "inMulitSelect", (!inMulitSelect));
                    if((!inMulitSelect) === false){
                        playlistView.delegateModelGroup.pop();
                        playlistView.dragGroup.pop();
                    }else{
                        playlistView.delegateModelGroup.push(playlistView.lastIndex);
                        playlistView.dragGroup.push(listmodel.get(playlistView.lastIndex).coverUrl)
                    }
                }else{
                    playlistView.checkOne(playlistView.lastIndex);
                    playlistView.currentIndex = playlistView.lastIndex;
                }
            }

            function checkOne(idx){
                playlistView.removeModelGroup();
                listmodel.setProperty(idx, "inMulitSelect", true);
                playlistView.delegateModelGroup.push(idx);
                playlistView.dragGroup.push(listmodel.get(idx).coverUrl);
                playlistView.lastIndex = idx;
            }

            function checkMulti(idx){
                playlistView.removeModelGroup();
                var beging = (playlistView.lastIndex >= idx) ? idx : playlistView.lastIndex
                var end = (playlistView.lastIndex >= idx) ? playlistView.lastIndex: idx
                for(var i = beging; i <= end; i++){
                    listmodel.setProperty(i, "inMulitSelect", true);
                    playlistView.delegateModelGroup.push(i);
                    playlistView.dragGroup.push(listmodel.get(i).coverUrl);
                }
                playlistView.lastIndex = idx;
            }

            function getSelectGroupHashList(){
                var hashList = [];
                for(var j = 0; j < playlistView.delegateModelGroup.length; j++){
                    if(listmodel.get(playlistView.delegateModelGroup[j]).inMulitSelect){
                        var tmpHash  = listmodel.get(playlistView.delegateModelGroup[j]).hash;
                        hashList.push(tmpHash);
                    }
                }
                return hashList;
            }

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
                    playlistView.lastIndex--;
                    if(isShiftModifier && keyChanged === 2) playlistView.lastIndex++
                    keyChanged = 1;
                    playlistView.keysShiftModifier();
                    break;
                case Qt.Key_Down:
                    playlistView.lastIndex++;
                    if(isShiftModifier && keyChanged === 1) playlistView.lastIndex--
                    keyChanged = 2;
                    playlistView.keysShiftModifier();
                    break;
                case Qt.Key_A:
                    if (event.modifiers & Qt.ControlModifier) {
                        playlistView.lastIndex = 0;
                        playlistView.checkMulti(listmodel.count -1);
                    }
                    break;
                case Qt.Key_Shift:
                    playlistView.isShiftModifier = true;
                    break;
                case Qt.Key_Delete:
                    playlistView.delectSelectMusices();
                    break;
                case Qt.Key_L:
                    if (event.modifiers & Qt.ControlModifier) {
                        infoDialog.musicData = listmodel.get(lastIndex);
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
                    playlistView.isShiftModifier = false;
                }
            }
        }
    }

    NumberAnimation on x {
        id: playlistRaiseAnimation
        running: false
        from: rootWindow.width
        to: rootWindow.width - width - 10
        duration: 500
        easing.type: Easing.OutQuart
    }
    NumberAnimation on x {
        id: playlistHideAnimation
        running: false
        from: rootWindow.width - width - 10
        to: rootWindow.width
        duration: 300
        easing.type: Easing.OutQuart
        onStopped: playlistHided()
    }

    Connections {
        target: playlistHideAnimation
        onStopped: {
            isPlaylistShow = false
        }
    }

    function onMousePressed(x, y) {
        var object = playlistRoot.mapFromGlobal(x, y)
        if (isPlaylistShow && !playlistRoot.contains(object)) {
            playlistHideAnimation.start()
        }
    }
    function playlistRaise(){
        if (!isPlaylistShow) {
            playlistRaiseAnimation.start()
        }
    }

    Component.onCompleted: {
        EventsFilter.mousePress.connect(onMousePressed)
        globalVariant.playlistExist = true
    }
}
