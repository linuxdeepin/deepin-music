// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0
import audio.global 1.0
import "../allItems"

ItemDelegate{
    property double scalingratio: 168 / 810   //计算宽度占比
    property string delegateListHash: ""
    property string playbackAlbumName: ""
    property bool playbackByArtist: false
    property bool showCoverImage: true
    property bool showTrackNumber: true
    property bool showArtistColumn: true
    property bool indexPlaybackState: false
    readonly property bool activeMeta: globalVariant.curPlayingHash === hash
    readonly property bool playing: globalVariant.curPlayingStatus === DmGlobal.Playing
    readonly property int leadingColumnWidth: showTrackNumber
                                              ? (indexPlaybackState ? 72 : 56)
                                              : 40
    readonly property int extendedColumnCount: showExtendedColumns
                                                ? (showArtistColumn ? 2 : 1)
                                                : 0
    property bool isDragged: false
    property int hoverY: height
    property string hashList: ""
    // 判断是否显示扩展列（Artist, Album）
    property bool showExtendedColumns: listview.width > 500

    id: rootRectangle
    checked: inMulitSelect
    hoverEnabled: true
    contentItem: Item {}
    indicator: Item {}

    function playCurrent() {
        if (playbackByArtist)
            Presenter.playArtist(artist, hash)
        else if (playbackAlbumName !== "")
            Presenter.playAlbum(playbackAlbumName, hash)
        else
            Presenter.playPlaylist(delegateListHash, hash)
        imagecell.setplayActionButtonIcon("list_pussed")
    }

    function toggleCurrentPlayback() {
        if (activeMeta && playing)
            Presenter.pause()
        else
            playCurrent()
    }

    function unfavoriteIconColor() {
        if (rootRectangle.checked)
            return rootRectangle.palette.highlightedText
        return rootRectangle.palette.windowText
    }

    anchors.horizontalCenter: listview.contentItem.horizontalCenter

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
        playCurrent()
    }
    Keys.onEnterPressed: {
        playCurrent()
    }

    MouseArea {
        id: mouseArea
        anchors.fill: rootRectangle
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        drag.target: rootRectangle

        onPressed: function(mouse) {
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
        onReleased: function(mouse) {
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
            playCurrent()
        }
    }
    Component {
        id: hoverbuttons
        Row {
            // The visible 16px icons are 10px apart.  Their MouseAreas are
            // enlarged independently so the visual gap is not inflated by
            // two 32px layout slots.
            width: 42
            height: 32
            spacing: 10
            Item {
                id: addButton
                width: 16
                height: 32
                DciIcon {
                    anchors.centerIn: parent
                    width: 16; height: 16
                    sourceSize: Qt.size(16, 16)
                    theme: DTK.themeType
                    palette: DTK.makeIconPalette(rootRectangle.palette)
                    name: rootRectangle.checked ? "list_add_checked" : "list_add"
                }
                MouseArea {
                    width: 24
                    height: 32
                    anchors.centerIn: parent
                    onClicked: {
                        var tmpHash = []
                        tmpHash.push(model.hash)
                        if (importMenuLoader.status === Loader.Null) {
                            importMenuLoader.setSource("../musicmousemenu/ImportMenu.qml")
                            importMenuLoader.item.pageHash = viewListHash
                        }
                        if (importMenuLoader.status === Loader.Ready) {
                            importMenuLoader.item.mediaHashList = tmpHash
                            importMenuLoader.item.itemIndex = index
                            importMenuLoader.item.popup()
                        }
                    }
                }
            }
            Item {
                id: moreButton
                width: 16
                height: 32
                DciIcon {
                    anchors.centerIn: parent
                    width: 16; height: 16
                    sourceSize: Qt.size(16, 16)
                    theme: DTK.themeType
                    palette: DTK.makeIconPalette(rootRectangle.palette)
                    name: rootRectangle.checked ? "list_more_checked" : "list_more"
                }
                MouseArea {
                    width: 24
                    height: 32
                    anchors.centerIn: parent
                    onClicked: {
                        if (moreMenuLoader.status === Loader.Null) {
                            moreMenuLoader.setSource("../musicmousemenu/MusicMoreMenu.qml")
                            moreMenuLoader.item.pageHash = viewListHash
                            moreMenuLoader.item.mediaData = model
                        }
                        if (moreMenuLoader.status === Loader.Ready) {
                            moreMenuLoader.item.mediaData = model
                            moreMenuLoader.item.itemIndex = index
                            moreMenuLoader.item.popup()
                        }
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
            width: rootRectangle.leadingColumnWidth; height: 56
            color: Qt.rgba(0, 0, 0, 0)
            Row {
                visible: rootRectangle.showTrackNumber && !rootRectangle.indexPlaybackState
                anchors.centerIn: parent
                leftPadding: 10
                spacing: 10
                Label {
                    id: numlabel
                    elide: Text.ElideRight
                    text: (index+1 < 10) ? "0%1".arg(index + 1) : index+1      //index+1
                    anchors.verticalCenter: parent.verticalCenter
                }
                Item {
                    id: heartbutton
                    width: 32
                    height: 32
                    anchors.verticalCenter: numlabel.verticalCenter
                    ActionButton {
                        anchors.fill: parent
                        visible: favourite
                        hoverEnabled: false
                        icon.name: "heart_check"
                        icon.width: 16
                        icon.height: 16
                        palette.windowText: "#F75B5B"
                    }
                    DciIcon {
                        id: numberedFavoriteIcon
                        anchors.centerIn: parent
                        width: 16; height: 16
                        sourceSize: Qt.size(16, 16)
                        theme: DTK.themeType
                        palette: DTK.makeIconPalette(rootRectangle.palette)
                        visible: false
                        name: "heart"
                    }
                    ColorOverlay {
                        anchors.fill: numberedFavoriteIcon
                        source: numberedFavoriteIcon
                        color: rootRectangle.unfavoriteIconColor()
                        visible: !favourite
                        cached: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (mmType !== DmGlobal.MimeTypeCDA) {
                                if (favourite === false) {
                                    Presenter.addMetasToPlayList(hash, "fav")
                                } else {
                                    Presenter.removeFromPlayList(hash, "fav")
                                    globalVariant.sendFloatingMessageBox(qsTr("My Favorites"), 2)
                                }
                            }
                        }
                    }
                }
            }

            Row {
                visible: rootRectangle.showTrackNumber && rootRectangle.indexPlaybackState
                anchors.fill: parent

                Item {
                    width: 32
                    height: parent.height

                    Label {
                        anchors.centerIn: parent
                        visible: !rootRectangle.hovered && !rootRectangle.activeMeta
                        text: (index + 1 < 10) ? "0%1".arg(index + 1) : index + 1
                    }

                    Item {
                        width: 32
                        height: 32
                        anchors.centerIn: parent
                        visible: rootRectangle.hovered || rootRectangle.activeMeta

                        DciIcon {
                            id: indexPlaybackIcon
                            anchors.centerIn: parent
                            width: 16; height: 16
                            sourceSize: Qt.size(16, 16)
                            visible: false
                            name: rootRectangle.activeMeta && rootRectangle.playing
                                  ? globalVariant.playingIconName
                                  : "list_play"
                        }
                        ColorOverlay {
                            anchors.fill: indexPlaybackIcon
                            source: indexPlaybackIcon
                            color: rootRectangle.checked
                                   ? rootRectangle.palette.highlightedText
                                   : rootRectangle.palette.highlight
                            cached: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: rootRectangle.toggleCurrentPlayback()
                        }
                    }
                }

                Item {
                    width: 40
                    height: parent.height

                        Item {
                            width: 32
                            height: 32
                            anchors.centerIn: parent
                        ActionButton {
                            anchors.fill: parent
                            visible: favourite
                            hoverEnabled: false
                            icon.name: "heart_check"
                            icon.width: 16
                            icon.height: 16
                            palette.windowText: "#F75B5B"
                        }
                        DciIcon {
                            id: indexedFavoriteIcon
                            anchors.centerIn: parent
                            width: 16; height: 16
                            sourceSize: Qt.size(16, 16)
                            theme: DTK.themeType
                            palette: DTK.makeIconPalette(rootRectangle.palette)
                            visible: false
                            name: "heart"
                        }
                        ColorOverlay {
                            anchors.fill: indexedFavoriteIcon
                            source: indexedFavoriteIcon
                            color: rootRectangle.unfavoriteIconColor()
                            visible: !favourite
                            cached: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (mmType !== DmGlobal.MimeTypeCDA) {
                                    if (favourite === false)
                                        Presenter.addMetasToPlayList(hash, "fav")
                                    else {
                                        Presenter.removeFromPlayList(hash, "fav")
                                        globalVariant.sendFloatingMessageBox(qsTr("My Favorites"), 2)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Item {
                visible: !rootRectangle.showTrackNumber
                anchors.fill: parent

                Item {
                    width: 32
                    height: 32
                    anchors.centerIn: parent
                    ActionButton {
                        anchors.fill: parent
                        visible: favourite
                        hoverEnabled: false
                        icon.name: "heart_check"
                        icon.width: 16
                        icon.height: 16
                        palette.windowText: "#F75B5B"
                    }
                    DciIcon {
                        id: unnumberedFavoriteIcon
                        anchors.centerIn: parent
                        width: 16; height: 16
                        sourceSize: Qt.size(16, 16)
                        theme: DTK.themeType
                        palette: DTK.makeIconPalette(rootRectangle.palette)
                        visible: false
                        name: "heart"
                    }
                    ColorOverlay {
                        anchors.fill: unnumberedFavoriteIcon
                        source: unnumberedFavoriteIcon
                        color: rootRectangle.unfavoriteIconColor()
                        visible: !favourite
                        cached: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (mmType !== DmGlobal.MimeTypeCDA) {
                                if (favourite === false)
                                    Presenter.addMetasToPlayList(hash, "fav")
                                else {
                                    Presenter.removeFromPlayList(hash, "fav")
                                    globalVariant.sendFloatingMessageBox(qsTr("My Favorites"), 2)
                                }
                            }
                        }
                    }
                }
            }
        }
        Rectangle {
            id: columnMusic
            // 窗口窄时，Title 列占据更多空间
            width: parent.width
                   - rootRectangle.extendedColumnCount * parent.width * scalingratio
                   - rootRectangle.leadingColumnWidth - 102
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
                    visible: rootRectangle.showCoverImage
                    width: visible ? 40 : 0
                    height: visible ? 40 : 0
                }
                Label {
                    id: musicNameLabel;
                    // Keep the two visible 16px icons 10px apart. Match the
                    // tighter trailing gutter used by the album list view.
                    width: rootRectangle.hovered || buttonsLoader.visible
                           ? columnMusic.width - (rootRectangle.showCoverImage ? 122 : 72)
                           : columnMusic.width - (rootRectangle.showCoverImage ? 80 : 30)
                    height: 17
                    elide: Text.ElideRight
                    text: title
                    anchors.verticalCenter: imagecell.verticalCenter
                    verticalAlignment: Qt.AlignVCenter
                    palette.text: DTK.themeType === ApplicationHelper.DarkType ? "#B2F7F7F7" : "#000000"
                    color: checked ? palette.highlightedText :
                                     (imagecell.isCurPlay ? palette.highlight : palette.text)
                    font: DTK.fontManager.t7
                }
                Loader {
                    id: buttonsLoader;
                    width: 42
                    height: 32
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
            visible: showExtendedColumns && rootRectangle.showArtistColumn
        }

        Label {
            id: ablumLabel
            width: parent.width * scalingratio
            height: 56
            elide: Text.ElideRight
            text: (album === "") ? "undefind": album
            anchors.verticalCenter: parent.verticalCenter
            verticalAlignment: Qt.AlignVCenter
            visible: showExtendedColumns  // 窗口窄时隐藏
        }

        Label {
            id: musictimeLabel
            width: 102
            height: 56
            leftPadding: 10
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
