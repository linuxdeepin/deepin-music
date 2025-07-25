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
    property var palColor: DTK.palette.highlight

    // 动态对象池：根据实际需求创建和扩展
    property var pathCurvePool: []
    property bool poolInitialized: false

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
            // 只有在有数据时才重新绘制波形
            if (parent.pointList && parent.pointList.length > 0) {
                onAudioDataChanged(parent.pointList)
            }
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
        property color startColor: waveItem.stripedStartColor
        property color endColor: waveItem.stripedEndColor
        color: "transparent"
        LinearGradient {
            id: linearGradient
            anchors.fill: parent
            visible: false
            start: Qt.point(0, 0)
            end: Qt.point(parent.width, 0)
            gradient: Gradient {
                GradientStop { position: 0.0; color: shadowRect.startColor}
                GradientStop { position: 1.0; color: shadowRect.endColor}
            }
        }
        FastBlur {
            id: blur
            width: totalSecs > 0 ? curSecs * parent.width / totalSecs : 0
            height: linearGradient.height
            anchors.left: linearGradient.left
            anchors.verticalCenter: linearGradient.verticalCenter
            source: linearGradient
            radius: 48
            transparentBorder: true
            cached: true
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

    // 从对象池获取或创建PathCurve对象
    function getPathCurve(index) {
        if (index < pathCurvePool.length) {
            return pathCurvePool[index]
        } else {
            // 对象池中没有足够的对象，创建新的并加入池中
            var newCurve = Qt.createQmlObject('import QtQuick 2.11; PathCurve {}', maskPath);
            pathCurvePool.push(newCurve)
            return newCurve
        }
    }

    // 初始化对象池（基于第一次实际需求）
    function initializePool(requiredSize) {
        if (poolInitialized) return
        
        console.warn("Initializing PathCurve object pool with", requiredSize, "objects based on actual requirements")
        pathCurvePool = []
        for (var i = 0; i < requiredSize; i++) {
            var curve = Qt.createQmlObject('import QtQuick 2.11; PathCurve {}', maskPath);
            pathCurvePool.push(curve)
        }
        poolInitialized = true
        console.warn("PathCurve object pool initialized successfully with", pathCurvePool.length, "objects")
    }

    function onAudioDataChanged(pointList) {
        // 如果没有传递参数，使用 parent.pointList 作为后备
        if (pointList === undefined) {
            pointList = parent.pointList || []
        }
        
        // 清理原有的pathElements，但不销毁对象，只是清空数组
        maskPath.pathElements = []
        
        var pointCount = Math.floor(shapeMask.width / waveItem.sampleRectWidth / magnification)
        if (pointCount <= 0) pointCount = 1  // 确保至少有1个点
        
        var step = Math.floor(pointList.length / pointCount)
        if (step < 1) step = 1  // 确保step至少为1
        
        
        // 第一次使用时初始化对象池
        if (!poolInitialized) {
            var requiredObjects = (pointList.length === 0) ? 3 : (pointCount + 1)
            initializePool(requiredObjects)
        }
        
        if (pointList.length === 0) {
            console.warn("Creating static curves for empty pointList")
            var startCurve = getPathCurve(0)
            startCurve.x = 0
            startCurve.y = shapeMask.height - 2
            maskPath.pathElements.push(startCurve)

            var rightTopCurve = getPathCurve(1)
            rightTopCurve.x = shapeMask.width
            rightTopCurve.y = shapeMask.height - 2
            maskPath.pathElements.push(rightTopCurve)

            var endCurve = getPathCurve(2)
            endCurve.x = shapeMask.width
            endCurve.y = shapeMask.height
            maskPath.pathElements.push(endCurve)
        } else {
            for(var i = 0; i < pointCount; i++) {
                var pathcurve = getPathCurve(i)

                pathcurve.x = waveItem.sampleRectWidth * magnification * i
                pathcurve.y = shapeMask.height - Math.ceil(pointList[i * step] *  waveItem.height)

                if (pathcurve.y <= 0)
                    pathcurve.y = 1
                if (pathcurve.y >= shapeMask.height)
                    pathcurve.y = shapeMask.height - 2
                maskPath.pathElements.push(pathcurve)
            }
            var endCurve = getPathCurve(pointCount);
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

    function updateStriped(baseColor) {
        var index = -1
        var accentColor = [("#df4187"),
                           ("#ff5d00"),
                           ("#f8cb00"),
                           ("#23c400"),
                           ("#00a48a"),
                           ("#0081ff"),
                           ("#3c02ff"),
                           ("#8c00d4"),
                           ("#4d4d4d")]

        for (var i = 0; i < accentColor.length; i++) {
            if (Qt.colorEqual(accentColor[i], palColor)) {
                index = i;
                break;
            }
        }
        if (index != -1) {
            var waveColor = [{start: Qt.rgba(199/255, 0, 70/255, 1), end: Qt.rgba(206/255, 166/255, 255/255, 1)},
                             {start: Qt.rgba(216/255, 54/255, 0, 1), end: Qt.rgba(255/255, 234/255, 165/255, 1)},
                             {start: Qt.rgba(216/255, 139/255, 0, 1), end: Qt.rgba(254/255, 255/255, 45/255, 1)},
                             {start: Qt.rgba(66/255, 134/255, 0, 1), end: Qt.rgba(99/255, 255/255, 162/255, 1)},
                             {start: Qt.rgba(0, 141/255, 36/255, 1), end: Qt.rgba(77/255, 255/255, 191/255, 1)},
                             {start: Qt.rgba(2/255, 6/255, 205/255, 1), end: Qt.rgba(60/255, 230/255, 255/255, 1)},
                             {start: Qt.rgba(59/255, 20/255, 194/255, 1), end: Qt.rgba(248/255, 142/255, 255/255, 1)},
                             {start: Qt.rgba(111/255, 0, 149/255, 1), end: Qt.rgba(255/255, 112/255, 205/255, 1)},
                             {start: Qt.rgba(0, 0, 0, 1), end: Qt.rgba(132/255, 132/255, 132/255, 1)}]
            waveItem.stripedStartColor = waveColor[index].start
            waveItem.stripedEndColor = waveColor[index].end
        } else {
            waveItem.stripedStartColor = "#0206cd"
            waveItem.stripedEndColor = "#3ce6ff"
        }
    }
    onPalColorChanged: {
        updateStriped(palColor)
    }
}
