/*
 * Copyright (C) 2022 UnionTech Technology Co., Ltd.
 *
 * Author:     yeshanshan <yeshanshan@uniontech.com>
 *
 * Maintainer: yeshanshan <yeshanshan@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.11
import QtQuick.Controls 2.4
import audio.global 1.0
import org.deepin.dtk 1.0

Rectangle {
    id: control
    property alias source: image.source
//    property size imageSourceSize
    property bool m_isPlaying: (globalVariant.curPlayingStatus === DmGlobal.Playing) ? true : false
//    property real imageScale: 1
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


    ActionButton {
        id: playActionButton
        anchors.centerIn: image
        icon.name: globalVariant.playingIconName
        icon.width: 20
        icon.height: 20
        visible: control.isCurPlay
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
//    hoverEnabled: true;
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
