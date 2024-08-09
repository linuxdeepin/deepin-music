// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import "../allItems"

Rectangle {
    property int itemfixedwidth: 208  //ui固定
    property int itemfixedheight: 230
    property bool playing: false
    property bool activeMeta:(globalVariant.curPlayingAlbum === name) ? true : false
    property int defaultY: -1
    signal itemDoubleClicked(var albumData)

    YAnimator {
        id: albumHoverItemAnimator
        target: rootrectangle
        from: defaultY
        to: defaultY - 10
        duration: 300
        easing.type: Easing.InOutQuad
    }

    YAnimator {
        id: albumExitItemAnimator
        target: rootrectangle
        from: defaultY - 10
        to: defaultY
        duration: 300
        easing.type: Easing.InOutQuad
    }

    id: rootrectangle
    width: itemfixedwidth
    height: itemfixedheight
    color: Qt.rgba(0, 0, 0, 0)
    MouseArea {
        anchors.fill: rootrectangle;
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        onDoubleClicked: {
            itemDoubleClicked(model);
        }
        onClicked: {
            if(mouse.button === Qt.RightButton){
                albumMoreMenu.albumData = model
                albumMoreMenu.popup();
            }
        }
    }
    Column {
        id: albumColumn
        width: 168; height: 230
        anchors.centerIn: rootrectangle
        topPadding: 10
        spacing: 10
        Control {
            id: albumItem
            width: 168; height: 168
            hoverEnabled: true
            contentItem: RoundedImage {
                id: backgroundImage
                backgroundWidth: 168; backgroundHeight: 168
                backgroundImageUrl: "file:///" + coverUrl
                isCurHover: albumItem.hovered
                isCurPlay:activeMeta

                CircularButton {
                    width: 44
                    height: 44
                    anchors.centerIn: parent
                    iconName: playing && activeMeta ? "details_pussed" : "details_play"
                    visible: albumItem.hovered
                    onClicked: {
                        if(playing && activeMeta){
//                            iconName = "details_play"
                            Presenter.pause();
                        } else {
//                            iconName = "details_pussed"
                            Presenter.playAlbum(name);
                        }
                    }
                }

                ActionButton {
                    id: playbutton
                    anchors.centerIn: backgroundImage
                    icon.width: 40; icon.height: 40
                    icon.name: globalVariant.playingIconName //"details_playing"
                    visible: albumItem.hovered ? false : (activeMeta ? true : false)
                    palette.windowText: "white"
                    checked: false
                }
                CircularButton {
                    id: morebutton
                    anchors.right: backgroundImage.right;  anchors.rightMargin: 10
                    anchors.bottom: backgroundImage.bottom;  anchors.bottomMargin: 10
                    width: 24; height: 24
                    iconName: "details_more"
                    visible: albumItem.hovered
                    onClicked: {
                        albumMoreMenu.albumData = model
                        albumMoreMenu.popup();
                    }
                }
                Label {
                    id: numberLable
                    anchors.left: backgroundImage.left;  anchors.leftMargin: 10
                    anchors.bottom: backgroundImage.bottom;  anchors.bottomMargin: 10
                    width: 62; height: 17
                    elide: Text.ElideRight
                    text: musicCount === 1 ? qsTr("1 song") : qsTr("%1 songs").arg(musicCount);
                    font.pixelSize: 10
                    color: palette.highlightedText
                    visible: albumItem.hovered
                }
            }
            onHoveredChanged: {
                if (hovered) {
                    if (defaultY < 0) {
                        var p = albumColumn.mapToItem(rootrectangle.parent, albumColumn.x, albumColumn.y)
                        defaultY = p.y
                    }
                    albumHoverItemAnimator.start()
                } else
                    albumExitItemAnimator.start()
            }

//            background: Rectangle{
//                id: itemWidget
//                width: 168; height: 218
//                color: Qt.rgba(0,0,0,0)
//            }
        }
        Column {
            Label {
                width: 166; height: 20
                elide: Text.ElideRight
                text: artist
                font.pixelSize: 14
                color: DTK.themeType === ApplicationHelper.DarkType ? "#E5ffffff"
                                                                    : "#E5000000"
            }
            Label {
                width: 166; height: 17
                elide: Text.ElideRight
                text: name
                font.pixelSize: 12
                color: "#7C7C7C"
            }
        }
    }
}
