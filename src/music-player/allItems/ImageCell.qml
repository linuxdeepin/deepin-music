// Copyright (C) 2022 UnionTech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import audio.global 1.0
import org.deepin.dtk 1.0

Rectangle {
    id: control
    property alias source: image.source
    property bool m_isPlaying: (globalVariant.curPlayingStatus === DmGlobal.Playing) ? true : false
    signal clicked
    property string pageHash: ""
    property bool isCurPlay: false
    property bool isCurHover: false
    property var curMediaData
//    implicitWidth: childrenRect.width
//    implicitHeight: childrenRect.height

    width: 40; height: 40
    color: "transparent"

    Image {
        id: image
        width: parent.width; height: parent.height
        visible: false
        smooth: true
        antialiasing: true
        Rectangle {
            id: curMask
            anchors.fill: parent
            color: isCurPlay || isCurHover ? Qt.rgba(0, 0, 0, 0.5) : Qt.rgba(0, 0, 0, 0)
        }
    }
    Rectangle {
        id: mask
        anchors.fill: parent
        radius: 3
        visible: false
    }
    OpacityMask {
        anchors.fill: parent
        source: image
        maskSource: mask
    }

    // border
    Rectangle {
        id: borderRect
        anchors.fill: parent
        color: "transparent"
        border.color: Qt.rgba(0, 0, 0, 0.1)
        border.width: 1
        visible: true
        radius: 3
    }

    ActionButton {
        id: playActionButton
        anchors.centerIn: image
        icon.name: globalVariant.playingIconName
        icon.width: 20
        icon.height: 20
        visible: control.isCurPlay
        palette.windowText: "white"
        onClicked:{
            if(control.m_isPlaying && control.isCurPlay){
                icon.name = "list_play"
                Presenter.pause();
            }else{
                icon.name = "list_pussed"
                if(control.pageHash === "album"){
                    Presenter.playAlbum(curMediaData.name);
                }else if(control.pageHash === "artistSublist"){
                    Presenter.playArtist(curMediaData.artist, curMediaData.hash);
                }else if(control.pageHash === "fav"){
                    Presenter.playPlaylist(pageHash, curMediaData.hash);
                }else if(control.pageHash === "play"){
                    if(globalVariant.curPlayingHash !== curMediaData.hash)
                        Presenter.setActivateMeta(curMediaData.hash)
                    Presenter.play()
                }else{
                    Presenter.playPlaylist(control.pageHash, curMediaData.hash);
                }
            }
       }
    }

    function itemHoveredChanged(value) {
        if(value === true){
            playActionButton.visible = true;
            if(control.m_isPlaying && control.isCurPlay){
                playActionButton.icon.name = "list_pussed";
            }else{
                playActionButton.icon.name = "list_play";
            }
        } else {
            playActionButton.visible = control.isCurPlay;
            playActionButton.icon.name = Qt.binding(function(){return globalVariant.playingIconName});
            return;
        }
    }
    function setplayActionButtonIcon(value){playActionButton.icon.name = value}
    onIsCurPlayChanged: {
        playActionButton.visible = control.isCurPlay
    }
}
