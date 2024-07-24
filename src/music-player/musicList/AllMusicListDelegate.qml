// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import audio.global 1.0
import "../allItems"

ItemDelegate{
    property double scalingratio: 168 / 810   //计算宽度占比
    property string delegateListHash: ""
    property bool isDragged: false
    property int hoverY: height
    property string hashList: ""

    id: rootRectangle
    anchors.horizontalCenter: parent.horizontalCenter
    checked: inMulitSelect
    hoverEnabled: true

    Drag.active: mouseArea.drag.active
    Drag.supportedActions: Qt.MoveAction
    Drag.dragType: Drag.Automatic
    Drag.mimeData: {
        "music-list/index-list": listview.delegateModelGroup,
        "music-list/hash-list": hashList
    }
    Drag.hotSpot.x: -15
    Drag.hotSpot.y: -15
    Drag.onDragFinished: {
        isDragged = true
    }

    Keys.onReturnPressed: {
        Presenter.playPlaylist(delegateListHash, hash);
        imagecell.setplayActionButtonIcon("list_pussed");
    }
    Keys.onEnterPressed: {
        Presenter.playPlaylist(delegateListHash, hash);
        imagecell.setplayActionButtonIcon("list_pussed");
    }

    MouseArea {
        id: mouseArea
        anchors.fill: rootRectangle
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        drag.target: rootRectangle

        onPressed: {
            if (mouse.button ===  Qt.LeftButton){
                listview.forceActiveFocus();
                listview.currentIndex = index
                var inMulitSelect = mediaModel.get(index).inMulitSelect;
                globalVariant.currentSelectMediaMeta = mediaModel.get(index)

                switch(mouse.modifiers){
                case Qt.ControlModifier:
                    mediaModel.setProperty(index, "inMulitSelect", (!inMulitSelect));
                    listview.delegateModelGroup.push(index);
                    listview.dragGroup.push(coverUrl)
                    break;
                case Qt.ShiftModifier:
                    listview.checkMulti(index);
                    break;
                default:
                    if (!inMulitSelect)
                        listview.checkOne(index);
                    dragDelegate.updateImages(listview.dragGroup)
                    var list = []
                    for (var i = 0; i < listview.delegateModelGroup.length; i++){
                        list.push(mediaModel.get(listview.delegateModelGroup[i]).hash);
                    }
                    hashList = list.join(",")
                    dragDelegate.grabToImage(function(result) {
                        //result.saveToFile("test1.png")
                        parent.Drag.imageSource = result.url
                    }/*, Qt.size(dragDelegate.width * globalVariant.devicePixelRatio,
                               dragDelegate.height * globalVariant.devicePixelRatio)*/);
                    break;
                }
            } else if (mouse.button ===  Qt.RightButton){
                if(listview.delegateModelGroup.length <= 1){
                    if (moreMenuLoader.status === Loader.Null ) {
                        console.log("moreMenuLoader...................")
                        moreMenuLoader.setSource("../musicmousemenu/MusicMoreMenu.qml")
                        moreMenuLoader.item.pageHash = viewListHash
                    }
                    if (moreMenuLoader.status === Loader.Ready ) {
                        console.log("moreMenuPopup...................")
                        moreMenuLoader.item.mediaData = model
                        moreMenuLoader.item.itemIndex = index
                        moreMenuLoader.item.popup();
                    }
                } else {
                    if (selectMenuLoader.status === Loader.Null ) {
                        selectMenuLoader.setSource("../musicmousemenu/MulitSelectMenu.qml")
                        selectMenuLoader.item.pageHash = viewListHash
                    }
                    if (selectMenuLoader.status === Loader.Ready ) {
                        selectMenuLoader.item.musicHashList = listview.getSelectGroupHashList()
                        selectMenuLoader.item.popup();
                    }
                }
            }
        }
        onReleased: {
            rootRectangle.x = 0
            if ((mouse.modifiers !== Qt.ShiftModifier && mouse.modifiers !== Qt.ControlModifier)
                    && mouse.button ===  Qt.LeftButton && !isDragged) {
                listview.checkOne(index);
                isDragged = false
            }
            if(parent.Drag.supportedActions === Qt.MoveAction && isDragged){
                //console.log("delegate onReleased: rootRectangle.y:", rootRectangle.y, "index:", index, "toIndex:", listview.dragToIndex)
                if (listview.dragToIndex < 0)
                    rootRectangle.y = 0 + listview.originY;
                else
                    rootRectangle.y = listview.dragToIndex * 56 + listview.originY;
                if (index > listview.dragToIndex || Presenter.playlistSortType(viewListHash) === DmGlobal.SortByCustom)
                    rootRectangle.y = index * 56 + listview.originY;
                mediaModel.setProperty(index, "dragFlag", false)
            }
            listview.dragToIndex = 0

        }
        onDoubleClicked: {
            Presenter.playPlaylist(delegateListHash, hash);
            imagecell.setplayActionButtonIcon("list_pussed");
        }
    }
    Component {
        id: hoverbuttons
        Row {
            spacing: 10
            ActionButton {
                id: addButton
                icon.name: rootRectangle.checked ? "list_add_checked" : "list_add"
                icon.width: 20; icon.height: 20
                onClicked: {
                    var tmpHash = [];
                    tmpHash.push(model.hash);
                    if (importMenuLoader.status === Loader.Null ) {
                        importMenuLoader.setSource("../musicmousemenu/ImportMenu.qml")
                        importMenuLoader.item.pageHash = viewListHash
                    }
                    if (importMenuLoader.status === Loader.Ready ) {
                        importMenuLoader.item.mediaHashList = tmpHash
                        importMenuLoader.item.itemIndex = index
                        importMenuLoader.item.popup();
                    }
                }
            }
            ActionButton {
                id: moreButton
                icon.name: rootRectangle.checked ? "list_more_checked" : "list_more"
                icon.width: 20; icon.height: 20
                onClicked: {
                    if (moreMenuLoader.status === Loader.Null ) {
                        moreMenuLoader.setSource("../musicmousemenu/MusicMoreMenu.qml")
                        moreMenuLoader.item.pageHash = viewListHash
                        moreMenuLoader.item.mediaData = model
                    }
                    if (moreMenuLoader.status === Loader.Ready ) {
                        moreMenuLoader.item.mediaData = model
                        moreMenuLoader.item.itemIndex = index
                        moreMenuLoader.item.popup();
                    }
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
        anchors.verticalCenter: parent.verticalCenter
        Rectangle {
            id: columnNumber
            width: 56; height: 56
            color: Qt.rgba(0, 0, 0, 0)
            Row {
                anchors.centerIn: parent
                leftPadding: 10
                spacing: 10
                Label {
                    id: numlabel
                    elide: Text.ElideRight
                    text: (index+1 < 10) ? "0%1".arg(index + 1) : index+1      //index+1
                    anchors.verticalCenter: parent.verticalCenter
                }
                ActionButton {
                    id: heartbutton
                    anchors.verticalCenter: numlabel.verticalCenter
                    icon.width: 20; icon.height: 20
                    icon.name: favourite ? "heart_check" : "heart"
                    palette.windowText: (favourite & !rootRectangle.checked)  ? "#F75B5B" : undefined
                    onClicked: {
                        if (mmType !== DmGlobal.MimeTypeCDA) {
                            if(favourite === false){
                                Presenter.addMetasToPlayList(hash, "fav")
                            } else {
                                Presenter.removeFromPlayList(hash, "fav")
                                globalVariant.sendFloatingMessageBox(qsTr("My Favorites"), 2);
                            }
                        }
                    }
                }
            }
        }
        Rectangle {
            id: columnMusic
            width: parent.width - 2 * parent.width * scalingratio - 158
            height: 56
            color: Qt.rgba(0, 0, 0, 0)
            Row {
                anchors.verticalCenter: columnMusic.verticalCenter
                spacing: 10
                leftPadding: 10
                ImageCell {
                    id: imagecell
                    source: "file:///" + coverUrl
                    pageHash: delegateListHash
                    isCurPlay: (globalVariant.curPlayingHash === hash) ? true : false
                    isCurHover: rootRectangle.hovered
                    curMediaData: model
                    width: 40; height: 40
                }
                Label {
                    id: musicNameLabel;
                    width: rootRectangle.hovered || buttonsLoader.visible ? columnMusic.width - 130:
                                                   columnMusic.width - 120;
                    height: 17
                    elide: Text.ElideRight
                    text: title
                    anchors.verticalCenter: imagecell.verticalCenter
                    palette.text: DTK.themeType === ApplicationHelper.DarkType ? "#B2F7F7F7" : "#000000"
                    color: checked ? palette.highlightedText :
                                     (imagecell.isCurPlay ? palette.highlight : palette.text)
                    font: DTK.fontManager.t7
                }
                Loader {
                    id: buttonsLoader;
                    anchors.verticalCenter: musicNameLabel.verticalCenter
                    sourceComponent: hoverbuttons
                    visible: rootRectangle.hovered ||
                             (importMenuLoader.status === Loader.Ready && importMenuLoader.item.visible && importMenuLoader.item.itemIndex === index) ||
                             (moreMenuLoader.status === Loader.Ready && moreMenuLoader.item.visible && moreMenuLoader.item.itemIndex === index)
                }
            }
        }
        Label {
            id: singerLabel
            width: parent.width * scalingratio
            height: 56
            leftPadding: 10
            elide: Text.ElideRight
            text: (artist === "") ? "undefind" : artist
            anchors.verticalCenter: parent.verticalCenter
            verticalAlignment: Qt.AlignVCenter
        }

        Label {
            id: ablumLabel
            width: parent.width * scalingratio
            height: 56
            elide: Text.ElideRight
            text: (album === "") ? "undefind": album
            anchors.verticalCenter: parent.verticalCenter
            verticalAlignment: Qt.AlignVCenter
        }

        Label {
            id: musictimeLabel
            width: 102
            height: 56
            elide: Text.ElideRight
            text:{
                var sec = Math.floor((length/1000)%60);
                if (sec < 10){
                    return Math.floor(length / 1000 / 60) + ":0" + sec
                }else{
                    return Math.floor(length / 1000 / 60) + ":" + sec
                }
            }
            anchors.verticalCenter: parent.verticalCenter
            verticalAlignment: Qt.AlignVCenter
        }
    }
}
    Rectangle {
        id: topDivider
        width: parent.width
        height: 1
        y: 0
        color: palette.highlight
        visible: index === 0 && listview.dragToIndex === -1 /*&& rootRectangle.hovered*/
    }

    Rectangle {
        id: bottomDivider
        width: parent.width
        height: 1
        y: parent.height - 1
        color: palette.highlight
        visible: dragFlag
    }

    onHoveredChanged: {
        imagecell.itemHoveredChanged(rootRectangle.hovered);
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
}
