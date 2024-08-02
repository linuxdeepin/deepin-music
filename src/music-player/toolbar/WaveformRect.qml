// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0
import "../allItems"

Rectangle {
    property int curSecs: 0
    property int totalSecs: 0
    property int magnification: 5
    property string dragTime: curSecs < 0 ? "00:00"
                                          : Math.floor(curSecs / 60) + (Math.floor(curSecs % 60) < 10 ? ":0" : ":") + Math.floor(curSecs % 60)

    property bool mousePressed: false
    property int hoverX: 0

    id: waveformRect
    implicitWidth: 360
    width: 360
    height: 60
    color: "#00000000"

    WaveformItem {
        id: waveItem
        width: parent.width
        height: 30
        anchors.centerIn: parent
        visible: false
    }
    Canvas {
        id: shapeMask
        width: parent.width
        height: waveItem.height
        anchors.centerIn: parent
        contextType: "2d"
        visible: false
        Path {
            id: maskPath
            startX: 0
            startY: shapeMask.height
        }

        onWidthChanged: {
            onAudioDataChanged()
            updatePosition(curSecs)
        }

        onPaint: {
            if(context !== null){
                context.clearRect(0, 0, width, height)

                context.path = maskPath;
                context.fill();
            }
        }
    }
    OpacityMask {
        anchors.fill: waveItem
        source: waveItem
        maskSource: shapeMask
    }

    Rectangle {
        id: shadowRect
        width: parent.width
        height: 3
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        color: "transparent"
        LinearGradient {
            id: linearGradient
            anchors.fill: parent
            visible: false
            start: Qt.point(0, 0)
            end: Qt.point(parent.width, 0)
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#0206cd"}
                GradientStop { position: 1.0; color: "#3ce6ff"}
            }
        }
        FastBlur {
            id: blur
            width: curSecs * parent.width / totalSecs
            height: linearGradient.height
            anchors.left: linearGradient.left
            anchors.verticalCenter: linearGradient.verticalCenter
            source: linearGradient
            radius: 48
            transparentBorder: true
        }
    }

    Rectangle {
        id: hoverTip
        width: 60
        height: parent.height + 20
        color: "transparent"
        visible: false
        x: hoverX - width / 2
        y: -25

        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            ArrowRectangle {
                id: timeLabel
                width: timeText.width > 50 ? timeText.width + 10 : 50
                height: 25
                fillColor: palette.highlight
                Text {
                    id: timeText
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: mousePressed ?  dragTime : currentTime
                    color: palette.highlightedText
                    font: DTK.fontManager.t8
                    verticalAlignment: Text.AlignHCenter
                }
            }
            Rectangle {
                width: 2
                height: 50
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#FFA500"
            }
            Triangle {
                width: 10
                height: 10
                fillColor: palette.highlight
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onPressed: {
            mousePressed = true
            curSecs = Math.floor(mouse.x / width * totalSecs)
            hoverX = mouseX
        }

        onReleased: {
            Presenter.setPosition(Math.ceil(mouse.x / width * totalSecs) * 1000 + 100)
            mousePressed = false
        }
        onPositionChanged: {
            if (mousePressed) {
                hoverX = mouseX
                if (hoverX < 0)
                    hoverX  =  0
                if (hoverX > width)
                    hoverX = width
                curSecs = Math.floor(hoverX / width * totalSecs)
            }
        }
        onEntered: {
            //console.log("onEntered:................" + songTitle.length + totalSecs)
            if (songTitle.length !== 0)
                hoverTip.visible = true
        }
        onExited: {
            hoverTip.visible = false
        }
    }

    function onAudioDataChanged() {
        maskPath.pathElements = []
        var pointCount = shapeMask.width / waveItem.sampleRectWidth / magnification
        var step = Math.floor(pointList.length / pointCount)
        if (pointList.length === 0) {
            var startCurve = Qt.createQmlObject('import QtQuick 2.11; PathCurve {}', maskPath);
            startCurve.x = 0
            startCurve.y = shapeMask.height - 2
            maskPath.pathElements.push(startCurve)

            var rightTopCurve = Qt.createQmlObject('import QtQuick 2.11; PathCurve {}', maskPath);
            rightTopCurve.x = shapeMask.width
            rightTopCurve.y = shapeMask.height - 2
            maskPath.pathElements.push(rightTopCurve)

            var endCurve = Qt.createQmlObject('import QtQuick 2.11; PathCurve {}', maskPath);
            endCurve.x = shapeMask.width
            endCurve.y = shapeMask.height
            maskPath.pathElements.push(endCurve)
        } else {
            //console.log("pointModel:" + pointList.length + "    pointCount:" + pointCount + "        step:" + step)
            for(var i = 0; i < pointCount; i++) {
                var pathcurve = Qt.createQmlObject('import QtQuick 2.11; PathCurve {}', maskPath);

                pathcurve.x = waveItem.sampleRectWidth * magnification * i
                pathcurve.y = shapeMask.height - Math.ceil(pointList[i * step] *  waveItem.height)

                if (pathcurve.y <= 0)
                    pathcurve.y = 1
                if (pathcurve.y >= shapeMask.height)
                    pathcurve.y = shapeMask.height - 2
                maskPath.pathElements.push(pathcurve)
            }
            var endCurve = Qt.createQmlObject('import QtQuick 2.11; PathCurve {}', maskPath);
            endCurve.x = shapeMask.width
            endCurve.y = shapeMask.height
            maskPath.pathElements.push(endCurve)
        }

        shapeMask.requestPaint()
    }
    function updatePosition(position) {
        if (!mousePressed) {
            curSecs = position
            hoverX = position * width / totalSecs
            //console.log("updatePosition:hoverX:" + hoverX + "  totalSecs:" + totalSecs + "  width:" + waveformRect.width + "  position:" + position)
        }
    }
}
