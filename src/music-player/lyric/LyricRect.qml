// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0

Rectangle {
    property int curIndex: 0
    property bool isFlicking: false
    property int itemHeight: 45
    property int highlightItemHeight: 60
    property real currentPosition: 0
    property var wordLyricsData: []

    id: lyricRect
    width: parent.width
    height: parent.height
    color: "#00000000"
    anchors.horizontalCenter: parent.horizontalCenter

    Timer{
        id: flickTimer
        interval: 3000
        repeat: false
        onTriggered: {
            isFlicking = false
        }
    }

    ListView {
        id: listViewLyric;
        anchors.fill: parent;
        clip: true
        ScrollBar.vertical: ScrollBar { }
        currentIndex: curIndex
        model: lrcModel
        delegate: lyricDelegate

        onFlickStarted: {
            isFlicking = true
        }
        onFlickEnded: {
            isFlicking = true
            if (flickTimer.running) {
                flickTimer.stop()
            }
            flickTimer.start()
        }
        onMovementStarted: {
            isFlicking = true
        }
        onMovementEnded: {
            if (flickTimer.running) {
                flickTimer.stop()
            }
            flickTimer.start()
        }

        function updateContentY() {
            if (isFlicking)
                return

            if(currentIndex * itemHeight <= lyricRect.height / 2) {
                //console.log("originY:" + originY + "     currentIndex:" + currentIndex + "      " )
                listViewLyric.contentY =  0 + originY;
            } else {
                if(currentIndex <= lrcModel.count - Math.round(lyricRect.height / 2 / itemHeight)) {
                    //console.log("originY:" + originY + "     contentY:" + contentY)
                    listViewLyric.contentY = (currentIndex - Math.round((lyricRect.height - /*highlightItemHeight*/ itemHeight) / itemHeight / 2)) * itemHeight + originY
                } else {
                    listViewLyric.contentY = (lrcModel.count - Math.round(lyricRect.height / itemHeight)) * itemHeight;
                }
            }
        }
        Behavior on contentY {
            NumberAnimation {
                duration: listViewLyric.currentIndex + 1 < listViewLyric.count - lyricRect.height / 2 / itemHeight ? 500 : 0
                easing.type: Easing.OutCubic
            }
        }
    }

    Component {
        id: lyricDelegate

        Rectangle {
            id: lyricItemRect
            width: parent ? parent.width : 0
            height: itemHeight
            color: "#00000000"
            property int lineIndex: index
            property bool hasWordTiming: model !== undefined && model.hasWordTiming === true
            property var lineWords: hasWordTiming ? wordLyricsData[lineIndex] : []

            // 距离相关的透明度值
            property real distanceOpacity: {
                if (lyricItemRect.ListView.isCurrentItem) {
                    return 1.0
                }
                if (curIndex <= Math.abs(lyricRect.height / itemHeight / 2)) {
                    if (index > Math.abs(lyricRect.height / itemHeight) - 2) {
                        return 0.24
                    } else if (index > Math.abs(lyricRect.height / itemHeight) - 3) {
                        return 0.42
                    } else {
                        return 0.7
                    }
                } else if (curIndex > lrcModel.count - Math.abs(lyricRect.height / itemHeight / 2)) {
                    if (index < lrcModel.count - Math.abs(lyricRect.height / itemHeight) + 1) {
                        return 0.24
                    } else if (index < lrcModel.count - Math.abs(lyricRect.height / itemHeight) + 2) {
                        return 0.42
                    } else {
                        return 0.7
                    }
                } else {
                    if (Math.abs(curIndex - index) > Math.abs(lyricRect.height / itemHeight / 2) - 1) {
                        return 0.24
                    } else if (Math.abs(curIndex - index) > Math.abs(lyricRect.height / itemHeight / 2) - 2) {
                        return 0.42
                    } else {
                        return 0.7
                    }
                }
            }

            // 普通歌词（无逐字时间轴）
            Text {
                id: txtLyric
                visible: !lyricItemRect.hasWordTiming
                width: parent ? parent.width : 0
                anchors.left: parent ? parent.left : undefined
                anchors.verticalCenter: parent ? parent.verticalCenter : undefined

                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                text: lyric
                color: {
                    if( lyricItemRect.ListView.isCurrentItem ) {
                        return palette.highlight
                    }
                    if(DTK.themeType === ApplicationHelper.LightType)
                        return Qt.rgba(0, 0, 0, distanceOpacity)
                    else
                        return Qt.rgba(255,255,255, distanceOpacity)
                }
                //font.family: "SourceHanSansSC"
                font.pixelSize: lyricItemRect.ListView.isCurrentItem ? 18 : 14
                font.weight: lyricItemRect.ListView.isCurrentItem ? Font.DemiBold : Font.Medium
            }

            // 逐字歌词（有逐字时间轴）
            Flow {
                id: wordFlow
                visible: lyricItemRect.hasWordTiming
                width: parent.width
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter

                Repeater {
                    id: wordRepeater
                    model: lyricItemRect.lineWords

                    Text {
                        property bool wordSung: modelData.time <= lyricRect.currentPosition

                        text: modelData.text
                        color: {
                            if (lyricItemRect.ListView.isCurrentItem) {
                                if (wordSung)
                                    return palette.highlight
                                else
                                    return DTK.themeType === ApplicationHelper.LightType
                                        ? Qt.rgba(0, 0, 0, 0.4)
                                        : Qt.rgba(255, 255, 255, 0.4)
                            } else {
                                if(DTK.themeType === ApplicationHelper.LightType)
                                    return Qt.rgba(0, 0, 0, distanceOpacity)
                                else
                                    return Qt.rgba(255,255,255, distanceOpacity)
                            }
                        }
                        Behavior on color {
                            ColorAnimation { duration: 150 }
                        }
                        font.pixelSize: lyricItemRect.ListView.isCurrentItem ? 18 : 14
                        font.weight: lyricItemRect.ListView.isCurrentItem ? Font.DemiBold : Font.Medium
                    }
                }
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                onDoubleClicked: {
                    console.log("onDoubleClicked:  index:" + index)
                    curIndex = index
                    var time = lrcModel.get(curIndex)["time"]
                    Presenter.setPosition(time)
                }
            }
        }
    }

    Connections {
        target: lrcRectItem
        onCurrentIndexChanged: {
            if (isFlicking)
                return
            if(!listViewLyric.flicking)
                curIndex = index

            listViewLyric.updateContentY()
        }
    }
}
