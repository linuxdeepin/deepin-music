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

Rectangle {
    property int itemfixedwidth: 208  //ui固定
    property int itemfixedheight: 196
    property bool playing: (globalVariant.curPlayingStatus === DmGlobal.Playing) ? true : false
    property bool activeMeta:(globalVariant.curPlayingArtist === name) ? true : false
    property int defaultY: -1
    signal itemDoubleClicked(var artistData)
    id: rootrectangle
    width: itemfixedwidth
    height: itemfixedheight
    color: Qt.rgba(0, 0, 0, 0)

    YAnimator {
        id: artistHoverItemAnimator
        target: rootrectangle
        from: defaultY
        to: defaultY - 10
        duration: 300
        easing.type: Easing.InOutQuad
    }

    YAnimator {
        id: artistExitItemAnimator
        target: rootrectangle
        from: defaultY - 10
        to: defaultY
        duration: 300
        easing.type: Easing.InOutQuad
    }

    MouseArea {
        anchors.fill: rootrectangle;
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        onDoubleClicked: {
            itemDoubleClicked(model);
        }
        onClicked: {
            if(mouse.button === Qt.RightButton){
                artistMoreMenu.artistData = model;
                artistMoreMenu.popup();
            }
        }
    }
    Column {
        id: artistColumn
        width: 148
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 6
        Control {
            id: cirWidgetControl
            width: 148; height: 148 //UI固定
            hoverEnabled: true
            contentItem: CircularImg {
                id:circular_img
                anchors.centerIn: cirWidget
                cuted_background: "file:///" + coverUrl
                background_width:cirWidget.width/2 + cirWidget.width/2
                background_height:cirWidget.height/2 + cirWidget.height/2
                isCurHover: cirWidgetControl.hovered
                isCurPlay: activeMeta

                CircularButton {
                    width: 40
                    height: 40
                    anchors.centerIn: parent
                    iconName: playing && activeMeta ? "details_pussed" : "details_play"
                    visible: cirWidgetControl.hovered
                    onClicked: {
                        if(playing && activeMeta){
                            Presenter.pause();
                        } else {
                            Presenter.playArtist(name);
                        }
                    }
                }
                ActionButton {
                    id: playbutton
                    anchors.centerIn: circular_img
                    icon.width: 40; icon.height: 40
                    icon.name: globalVariant.playingIconName //"details_playing"
                    visible: cirWidgetControl.hovered ?  false : (activeMeta ? true : false)
                    palette.windowText: "white"
                }

                CircularButton {
                    id: morebutton
                    anchors.right: circular_img.right;  anchors.rightMargin: 10
                    anchors.bottom: circular_img.bottom;  anchors.bottomMargin: 10
                    width: 24; height: 24
                    iconName: "details_more"
                    visible: cirWidgetControl.hovered
                    property Palette color: Palette {
                        normal: Qt.rgba(0, 0, 0, 0.1)
                        hovered: Qt.rgba(0, 0, 0, 0.15)
                    }
                    backgroundColor: color
                    onClicked: {
                        artistMoreMenu.artistData = model;
                        artistMoreMenu.popup();
                    }
                }
            }
            background: Rectangle {
                id: cirWidget
                width: 148; height: 148 //UI固定
                color: "transparent"
            }

            onHoveredChanged: {
                if (hovered) {
                    if (defaultY < 0) {
                        var p = artistColumn.mapToItem(rootrectangle.parent, artistColumn.x, artistColumn.y)
                        defaultY = p.y
                    }
                    artistHoverItemAnimator.start()
                } else {
                    artistExitItemAnimator.start()
                }
            }
        }
        Rectangle {
            width: parent.width
            height: 40
            color: "transparent"
            Column {
                width: parent.width
                Label {
                    width: 148; height: 20
                    text: (name === "") ? "undefind": name
                    elide: Text.ElideRight
                    font.pixelSize: 14
                    color: DTK.themeType === ApplicationHelper.DarkType ? "#E5ffffff"
                                                                        : "#E5000000"
                }
                Label {
                    width: 80; height: 17
                    text: musicCount === 1 ? qsTr("1 song") : qsTr("%1 songs").arg(musicCount);
                    font.pixelSize: 12
                    color: "#7C7C7C"
                }
            }
        }
    }
}
