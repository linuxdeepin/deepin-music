// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import Qt5Compat.GraphicalEffects
import audio.image 1.0
import audio.global 1.0

Rectangle {
        property url cuted_background
        property int background_width
        property int background_height
        property int arcWidth
        property color arcColor
        property color arcBackgroundColor
        property real  progress: 0
        property real  angle: 0.0

        id: img
        anchors.centerIn: parent
        width: background_width
        height: background_width
        radius: width/2
        color: "transparent"
        smooth: true
        View_image {
                id : _image
                visible: true
                width: parent.width-20
                height: parent.height-20
                anchors.centerIn: parent
                source: "noedgeimage"
                antialiasing: true
                rotation: angle
                smooth: true
                presenter: Presenter

            }
        FastBlur {
            anchors.top: _image.top; anchors.topMargin: 3
            anchors.left: _image.left; anchors.leftMargin: 1
            width: _image.width; height: _image.width
            source: _image
            radius: width/6
            transparentBorder: true
        }
        View_image {
                id : _image1
                visible: true
                width: parent.width
                height: parent.height
                anchors.centerIn: parent
                source: cuted_background
                antialiasing: true
                rotation: angle
                smooth: true
                presenter: Presenter
                signal sigPlaybackStatusChange(int playbackStatus)
                Timer{
                    id: timer
                    interval: 15
                    running: false
                    repeat: true
                    onTriggered: {
                        angle += 0.2;
                    }
               }
                function playbackStatusChange(playbackStatus){
                    if(playbackStatus === DmGlobal.Playing) {
                            timer.start()
                    } else {
                            timer.stop()
                    }
                }
                Component.onCompleted: {
                    Presenter.playbackStatusChanged.connect(sigPlaybackStatusChange)
                    _image1.sigPlaybackStatusChange.connect(playbackStatusChange)
                    if(Presenter.getPlaybackStatus() === DmGlobal.Playing){
                        timer.start();
                    }
                }
            }
    }
