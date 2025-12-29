// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0

Rectangle {
     //0 已存在，添加失败； 1 添加成功； 2 移除成功
    property int type: 0
    property real widthPadding: 15
    property real contentSpacing: 10
    property string message: "message"
    property real duration: 1000
    property real containerWidth: typeof rootWindow !== "undefined" ? rootWindow.width : parent.width
    property real containerHeight: typeof rootWindow !== "undefined" ? rootWindow.height : parent.height
    property real xOffset: typeof rootWindow !== "undefined" ? 110 : 0

    width: playPauseBtn.width + texteLabel.width + widthPadding * 2 + contentSpacing
    height: 60
    x: containerWidth / 2 - width / 2 + xOffset
    y: (containerHeight - height) / 2
    radius: 12
    color: {
        if(type === 1){
            return /*"#aa0081FF"*/ palette.highlight
        }else if(type === 2){
            return "#aa000000"
        }else{
            return /*"#aa0081FF"*/ palette.highlight
        }
    }
    visible: false

    Timer {
        id: hideTimer
        interval: duration
        repeat: false
        running: false
        onTriggered: {
            visible = false
        }
    }

    Row {
        width: parent.width
        height: parent.height
        spacing: contentSpacing
        leftPadding: widthPadding
        anchors.left: parent.left

        DciIcon {
            id: playPauseBtn
            name: "message_tips";
            sourceSize: Qt.size(21, 21)
            anchors.verticalCenter: parent.verticalCenter
        }
        Label {
            id: texteLabel
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr(message)
            color: palette.highlightedText
            textFormat: Text.PlainText
        }
    }
    function show() {
        visible = true
        if (hideTimer.running) {
            hideTimer.stop()
        }
        hideTimer.start()
    }
}
