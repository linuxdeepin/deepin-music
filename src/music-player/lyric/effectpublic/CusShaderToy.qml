// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import audio.global 1.0

ShaderEffect {
    id: shader

    //properties for shader

    //not pass to shader
    readonly property vector3d defaultResolution: Qt.vector3d(shader.width, shader.height, shader.width / shader.height)
    function calcResolution(channel) {
        if (channel) {
            return Qt.vector3d(channel.width, channel.height, channel.width / channel.height);
        } else {
            return defaultResolution;
        }
    }
    //pass
    readonly property vector3d  iResolution: defaultResolution
    property vector3d   iColor : Qt.vector3d(1.0, 0, 0)
    property real       iTime: 0
    property real       iTimeDelta: 100
    property int        iFrame: 10
    property real       iFrameRate
    property vector4d   iMouse;
    property var iChannel0  //only Image or ShaderEffectSource
    property var iChannel1  //only Image or ShaderEffectSource
    property var iChannel2; //only Image or ShaderEffectSource
    property var iChannel3; //only Image or ShaderEffectSource
    property var        iChannelTime: [0, 1, 2, 3]
    property var        iChannelResolution: [calcResolution(iChannel0), calcResolution(iChannel1), calcResolution(iChannel2), calcResolution(iChannel3)]
    property vector4d   iDate;
    property real       iSampleRate: 44100
    signal sigPlaybackStatusChange(int playbackStatus)
    function playbackStatusChange(playbackStatus){
        if(playbackStatus === DmGlobal.Playing) {
            timer1.start();
        } else {
            timer1.stop();
        }
    }
    Component.onCompleted: {
        Presenter.playbackStatusChanged.connect(sigPlaybackStatusChange)
        shader.sigPlaybackStatusChange.connect(playbackStatusChange)
        if(Presenter.getPlaybackStatus() === DmGlobal.Playing){
            timer1.start();
        }
    }

    //properties for Qml controller
    property alias hoverEnabled: mouse.hoverEnabled
    property bool running: true
    function restart() {
        shader.iTime = 0
        running = true
        timer1.restart()
    }
    Timer {
        id: timer1
        running: false
        triggeredOnStart: true
        interval: 16
        repeat: true
        onTriggered: {
            shader.iTime += 0.016;
        }
    }
    Timer {
        running: shader.running
        interval: 100
        onTriggered: {
            sigShaderStatusChange(status);
        }
    }

    Timer {
        running: shader.running
        interval: 1000
        onTriggered: {
            var date = new Date();
            shader.iDate.x = date.getFullYear();
            shader.iDate.y = date.getMonth();
            shader.iDate.z = date.getDay();
            shader.iDate.w = date.getSeconds()
        }
    }
    MouseArea {
        id: mouse
        anchors.fill: parent
        onPositionChanged: {
            shader.iMouse.x = mouseX
            shader.iMouse.y = mouseY
        }
        onClicked: {
            shader.iMouse.z = mouseX
            shader.iMouse.w = mouseY
        }
    }

    vertexShader: "qrc:/lyric/qsbfile/cusVertex.qsv"

    readonly property url defaultPixelShader: "qrc:/lyric/qsbfile/default.qsb"

    property url pixelShader: ""
    fragmentShader: pixelShader ? pixelShader : defaultPixelShader
}

