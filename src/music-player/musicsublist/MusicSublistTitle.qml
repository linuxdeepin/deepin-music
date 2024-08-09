// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0
import "../allItems"

Rectangle{
    property string pageHash: ""
    property int titleWidth
    property int titleHeight
    property var currentData
    id: titleRectangle
    width: titleWidth; height: titleHeight
//    color: "transparent"
    SublistTitleBackground{
        id: titleBackground
        width: titleWidth; height: titleHeight
        imageUrl: "file:///" + titleRectangle.currentData.coverUrl
        visible: titleButton1.visible
    }
    SublistTitleButton {
        id: titleButton1
        width: 703; height: 184
        anchors.left: titleRectangle.left; anchors.leftMargin: 20;
        anchors.bottom: titleRectangle.bottom; anchors.bottomMargin: 20
    }
    Control {
        id: titleButton2
        width: parent.width
        height: parent.height
        anchors.fill: parent/*; anchors.leftMargin: 24*/
        visible: false;
//        color: "transparent"
        background: Rectangle {
            anchors.fill: parent
            Image {
                id: img
                width: parent.width
                height: parent.height
                fillMode: Image.PreserveAspectCrop
                clip: true
                source: "file:///" + titleRectangle.currentData.coverUrl
            }
            FastBlur {
                anchors.fill: parent
                source: img
                radius: 64
            }
            Canvas {
                id: canvas
                width: parent.width
                height: parent.height
                smooth: true
                antialiasing: true
                onPaint: {
                    var ctx = getContext("2d");
                    var linerGradient = ctx.createLinearGradient(0, 0, 0, height);
                    linerGradient.addColorStop(0, Qt.rgba(248, 248, 248, 0.1))
                    linerGradient.addColorStop(0.5, Qt.rgba(248, 248, 248, 0.5))
                    linerGradient.addColorStop(1, Qt.rgba(248, 248, 248, 0.95))
                    ctx.clearRect(0, 0, width, height)
                    ctx.fillStyle = linerGradient
                    ctx.fillRect(0, 0, width, height);
                }
            }
        }

        Row {
            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 24
            spacing: 16
            DciIcon {
                id: playall
                anchors.verticalCenter: parent.verticalCenter
                name: "headline_play_bottom";
                sourceSize: Qt.size(34, 34)
                ActionButton {
                    anchors.fill: playall
                    icon.name: "list_play";
                    width: 20; height: 20
                    hoverEnabled: false;
                    onClicked: {
                        if (pageHash === "album") {
                            Presenter.playAlbum(currentData.name);
                        } else if(pageHash === "artist") {
                            Presenter.playArtist(currentData.name);
                        }
                    }
                }
            }
            Label {
                id: buttonLable
                text: currentData.name
                color: "#000000"
                font: DTK.fontManager.t2
            }
        }
    }
    function suspensionTitle(value){
        if(!value){
            titleButton1.visible = true;
            titleButton2.visible = false;
        }else{
            titleButton1.visible = false;
            titleButton2.visible = true;
        }
    }
}
