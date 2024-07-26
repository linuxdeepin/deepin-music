// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import audio.global 1.0
import "../allItems"
import "../dialogs"

ItemDelegate{
    property int imgCellRectWidth: 40
    property int timeRectWidth: 54
    property int spacingWidth: 10
    property bool isDragged: false

    id: rootRectangle
    checked: inMulitSelect
    width: 300
    height: 56
    hoverEnabled: true

    Drag.active: mouseArea.drag.active
    Drag.supportedActions: Qt.MoveAction
    Drag.dragType: Drag.Automatic
    Drag.mimeData: {
        "music-list/index-list": playlistView.delegateModelGroup
    }
    Drag.hotSpot.x: -15
    Drag.hotSpot.y: -15
    Drag.onDragFinished: {
        isDragged = true
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        drag.target: rootRectangle

        onDoubleClicked: {
            if(globalVariant.curPlayingHash !== hash)
                Presenter.setActivateMeta(hash)
            Presenter.play()
            imagecell.setplayActionButtonIcon("list_pussed");
        }
        onPressed: {
            if (mouse.button ===  Qt.LeftButton) {
                playlistView.forceActiveFocus()
                playlistView.currentIndex = index
                var inMulitSelect = listmodel.get(index).inMulitSelect;

                switch(mouse.modifiers){
                case Qt.ControlModifier:
                    listmodel.setProperty(index, "inMulitSelect", (!inMulitSelect));
                    playlistView.delegateModelGroup.push(index);
                    break;
                case Qt.ShiftModifier:
                    playlistView.checkMulti(index);
                    break;
                default:
                    if (!inMulitSelect)
                        playlistView.checkOne(index);
                    dragDelegate.updateImages(playlistView.dragGroup)
                    dragDelegate.grabToImage(function(result) {
                        parent.Drag.imageSource = result.url
                    });
                    break;
                }
            } else if (mouse.button ===  Qt.RightButton){
                if(playlistView.delegateModelGroup.length <= 1){
                    playlistMoreMenu.mediaData = model
                    playlistMoreMenu.itemIndex = index
                    playlistMoreMenu.popup()
                }else{
                    selectMenu.musicHashList = playlistView.getSelectGroupHashList();
                    selectMenu.popup();
                }
            }
        }
        onReleased: {
//            console.log("MouseArea,onReleased.............")
            rootRectangle.x = 0
            if ((mouse.modifiers !== Qt.ShiftModifier && mouse.modifiers !== Qt.ControlModifier)
                    && mouse.button ===  Qt.LeftButton && !isDragged) {
                playlistView.checkOne(index);
                isDragged = false
            }
            if(parent.Drag.supportedActions === Qt.MoveAction && isDragged){
                if (playlistView.dragToIndex < 0)
                    rootRectangle.y = 0 + playlistView.originY;
                else
                    rootRectangle.y = playlistView.dragToIndex * 56 + playlistView.originY;

                if (index > playlistView.dragToIndex)
                    rootRectangle.y = index * 56 + playlistView.originY;

                listmodel.setProperty(index, "dragFlag", false)
            }
            playlistView.dragToIndex = 0
        }
    }

    Component {
        id: hoverButtons
        Row {
            spacing: 4
            rightPadding: 10
            //layoutDirection: Qt.RightToLeft
            ActionButton {
                id: heartBtn
                icon.name: favourite ? "heart_check" : "playlist_heart"
                palette.windowText: (favourite & !rootRectangle.checked)  ? "#F75B5B" : undefined
                icon.width: 20
                icon.height: 20
                onClicked: {
                    var list = []
                    list.push(hash)
                    favourite ? Presenter.removeFromPlayList(list, "fav") : Presenter.addMetasToPlayList(list, "fav")
                    if(favourite === false)
                        globalVariant.sendFloatingMessageBox(qsTr("My Favorites"), 2)
                }
            }
            ActionButton {
                id: addBtn
                icon.name: rootRectangle.checked ? "playlist_add_checked" : "playlist_add"
                icon.width: 20
                icon.height: 20
                onClicked: {
                    playlistAddMenu.itemIndex = index
                    var tmpHash = [];
                    tmpHash.push(model.hash);
                    playlistAddMenu.mediaHashList = tmpHash;
                    playlistAddMenu.popup()
                }
            }
            ActionButton {
                id: delBtn
                icon.name: rootRectangle.checked ? "playlist_delete_checked" : "playlist_delete"
                icon.width: 20
                icon.height: 20
                onClicked: {
                    var list = []
                    list.push(hash)
                    Presenter.removeFromPlayList(list, "play", false)
                }
            }
            ActionButton {
                id: moreBtn
                icon.name: rootRectangle.checked ? "playlist_more_checked" : "playlist_more"
                icon.width: 20
                icon.height: 20
                onClicked: {
                    playlistMoreMenu.itemIndex = index
                    playlistMoreMenu.mediaData = model
                    playlistMoreMenu.popup()
                }
            }
        }
    }

    Rectangle {
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
        radius: 8
        color: rootRectangle.hovered ? Qt.rgba(0, 0, 0, 0.08) : Qt.rgba(0, 0, 0, 0)

        Row {
            width: parent.width
            height: parent.height
            spacing: 10
            leftPadding: 10
            anchors.centerIn: parent
            ImageCell {
                id: imagecell
                width: 40; height: 40
                anchors.verticalCenter: parent.verticalCenter
                source: "file:///" + coverUrl
                pageHash: "play"
                isCurPlay: (globalVariant.curPlayingHash === hash) ? true : false
                isCurHover: rootRectangle.hovered
                curMediaData: model
            }
            Rectangle {
                id: musicInfoRect
                width: rootRectangle.hovered || btnsLoader.visible ?
                           parent.width - imgCellRectWidth - 102 - spacingWidth * 3 :
                           parent.width - (imgCellRectWidth + timeRectWidth + spacingWidth * 2)
                height: 40
                anchors.verticalCenter: parent.verticalCenter
                color: "transparent"

                Column {
                    width: parent.width
                    height: parent.height
                    spacing: 1
                    anchors.centerIn: parent
                    Label {
                        id: musicNameLabel;
                        width: parent.width
                        height: 20
                        elide: Text.ElideRight
                        text: title
                        verticalAlignment: Qt.AlignVCenter
                        font: DTK.fontManager.t6
                        color: checked ? palette.highlightedText :
                                         (imagecell.isCurPlay ? palette.highlight : palette.text)
                    }
                    Label {
                        id: musicSingerLabel;
                        width: parent.width
                        height: 17
                        color: checked ? palette.highlightedText : "#7C7C7C"
                        elide: Text.ElideRight
                        text: artist
                        verticalAlignment: Qt.AlignVCenter
                        font: DTK.fontManager.t8
                    }
                }
            }
            Label {
                id: musicTimeLabel
                visible: !rootRectangle.hovered && !btnsLoader.visible
                width: timeRectWidth
                height: parent.height
                color: checked ? palette.highlightedText : "#7C7C7C"
                elide: Text.ElideRight
                text:{
                    var sec = Math.floor((length/1000)%60);
                    if (sec < 10){
                        return Math.floor(length / 1000 / 60) + ":0" + sec
                    }else{
                        return Math.floor(length / 1000 / 60) + ":" + sec
                    }
                }
                verticalAlignment: Qt.AlignVCenter
                anchors.verticalCenter: parent.verticalCenter
                font: DTK.fontManager.t8
            }
            Loader {
                id: btnsLoader
                anchors.verticalCenter: parent.verticalCenter
                sourceComponent: hoverButtons
                visible: rootRectangle.hovered || (playlistAddMenu.visible && playlistAddMenu.itemIndex === index) ||
                         (playlistMoreMenu.visible && playlistMoreMenu.itemIndex === index)
            }
        }

    }

    Rectangle {
        id: topDivider
        width: parent.width
        height: 1
        y: 0
        color: palette.highlight
        visible: index === 0 && playlistView.dragToIndex === -1
    }

    Rectangle {
        id: bottomDivider
        width: parent.width
        height: 1
        y: parent.height - 1
        color: palette.highlight
        visible: dragFlag
    }

    Rectangle {
        property int size: 0

        id: dragDelegate
        width: 70
        height: 70
        color: "transparent"
        visible: false

        Repeater {
            id:repeater
            model: 0

            DragItemDelegate {
                width: parent.width - 20
                height: parent.height - 20
                z: repeater.model.length - (index + 1)
                rotation: index * -20

                url: modelData
            }
        }

        Rectangle {
            width: 20
            height: 20
            z: 5
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 4
            anchors.bottomMargin: 4
            radius: 10
            color: "#ff0000"

            Text {
                id: txt
                anchors.centerIn: parent
                color: "#ffffff"
                font: DTK.fontManager.t8
                elide: Text.ElideMiddle
                text:dragDelegate.size
            }
        }

        function updateImages(imgList) {
            if (imgList.length > 3)
                repeater.model = imgList.slice(0, 3)
            else
                repeater.model = imgList.slice(0)
            size = imgList.length
        }

    }

    onHoveredChanged: {
        imagecell.itemHoveredChanged(rootRectangle.hovered);
    }
}
