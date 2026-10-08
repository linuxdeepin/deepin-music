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
        cache: false
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

    Item {
        id: playActionButton
        width: 32
        height: 32
        anchors.centerIn: image
        property alias iconName: playIcon.name
        visible: control.isCurPlay

        DciIcon {
            id: playIcon
            anchors.centerIn: parent
            width: 16
            height: 16
            sourceSize: Qt.size(16, 16)
            name: globalVariant.playingIconName
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (control.m_isPlaying && control.isCurPlay) {
                    playActionButton.iconName = "list_play"
                    Presenter.pause()
                } else {
                    playActionButton.iconName = "list_pussed"
                    if (control.pageHash === "album") {
                        Presenter.playAlbum(curMediaData.name)
                    } else if (control.pageHash === "artistSublist") {
                        Presenter.playArtist(curMediaData.artist, curMediaData.hash)
                    } else if (control.pageHash === "fav") {
                        Presenter.playPlaylist(pageHash, curMediaData.hash)
                    } else if (control.pageHash === "play") {
                        if (globalVariant.curPlayingHash !== curMediaData.hash)
                            Presenter.setActivateMeta(curMediaData.hash)
                        Presenter.play()
                    } else {
                        Presenter.playPlaylist(control.pageHash, curMediaData.hash)
                    }
                }
            }
        }
    }

    function itemHoveredChanged(value) {
        if(value === true){
            playActionButton.visible = true;
            if(control.m_isPlaying && control.isCurPlay){
                playActionButton.iconName = "list_pussed";
            }else{
                playActionButton.iconName = "list_play";
            }
        } else {
            playActionButton.visible = control.isCurPlay;
            playActionButton.iconName = Qt.binding(function(){return globalVariant.playingIconName});
            return;
        }
    }
    function setplayActionButtonIcon(value){playActionButton.iconName = value}
    onIsCurPlayChanged: {
        playActionButton.visible = control.isCurPlay
    }
}
