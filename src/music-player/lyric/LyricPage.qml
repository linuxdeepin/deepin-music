// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.2
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import Qt5Compat.GraphicalEffects

import org.deepin.dtk 1.0

Rectangle{
    id: lrcRectItem

    property string titleStr: ""
    property string artist: ""
    property string album: ""
    property string bgImgPath: "qrc:/dsg/img/music.svg"
    property bool withLrcs: lrcModel.count == 0 ? false : true
    property int centerAreaWidth: 899
//    property int curIndex: 0
    property ListModel lrcModel: ListModel{}

    signal currentIndexChanged(int index)

    Image {
        id: bgImg
        width: parent.width
        height: Window.height
        anchors.bottom: parent.bottom
        source: bgImgPath
        fillMode: Image.PreserveAspectCrop
        clip: true
        visible: false
        Rectangle {
            width: parent.width
            height: Window.height
            color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(0, 0, 0, 0.4) : Qt.rgba(247, 247, 247, 0.4)
        }
    }
    FastBlur {
        anchors.fill: bgImg
        source: bgImg
        radius: 128
    }

    Rectangle {
        width: centerAreaWidth
        height: parent.height
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"

    Row {
        spacing: 33
        width: parent.width
        height: parent.height
        topPadding: 70
        anchors.horizontalCenter: parent.horizontalCenter

        Rectangle {
            id: leftAreaColumn
            width: 426
            height: parent.height - 141
            color: "#00000000"

            Rectangle {
                id: leftArea
                width: 426
                height: 426
                color: "#00000000"
                anchors.centerIn: parent
                ShaderView {
                    id: shaderView
                }
            }
        }

        Column {
            id: rightAreaColumn

            property int lrcWidth: parent.width-630
            property int lrcHeigth: parent.height-164

            Rectangle {
                id: rightAreaRect
                width: 440
                height: rightAreaColumn.lrcHeigth
                color: "#00000000"
                visible: titleStr.length == 0 ? false : true

                Column {
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    Rectangle {
                        id: title
                        width: 440
                        height: 80
                        color: "#00000000"

                        Column {
                            width: parent.width
                            height: parent.height
                            spacing: 9

                            Rectangle {
                                width: parent.width
                                height: 35
                                color: "#00000000"

                                Label {
                                    width: parent.width
                                    text: titleStr
                                    color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(255, 255, 255, 0.9) : Qt.rgba(0, 0, 0, 0.9)
                                    font: DTK.fontManager.t3
                                    elide: Text.ElideRight
                                }
                            }
                            Rectangle {
                                width: parent.width
                                height: 20
                                color: "#00000000"
                                Row {
                                    width: parent.width
                                    height: parent.height
                                    spacing: 30

                                    Text {
                                        width: 174
                                        height: parent.height
                                        color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(255, 255, 255, 0.7) : Qt.rgba(0, 0, 0, 0.7)
                                        text: qsTr("Artist") + (": %1".arg(artist.length == 0 ? "未知" : artist));
                                        font: DTK.fontManager.t6
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        width: 174
                                        height: parent.height
                                        color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(255, 255, 255, 0.7) : Qt.rgba(0, 0, 0, 0.7)
                                        text: qsTr("Album") + ": %1".arg(album.length == 0 ? "未知" : album);
                                        font: DTK.fontManager.t6
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        visible: withLrcs
                        width: 440
                        height: rightAreaColumn.lrcHeigth - 100
                        color: "#00000000"
                        anchors.horizontalCenter: parent.horizontalCenter
                        LyricRect {
                            id: lyricRect
                        }
                    }
                    Rectangle {
                        id: nolyric
                        visible: !withLrcs
                        width: 440
                        height: rightAreaColumn.lrcHeigth - 150
                        color: "transparent"
                        anchors.horizontalCenter: parent.horizontalCenter
                        Column {
                            width: parent.width
                            height: parent.height
                            topPadding: 151 + (parent.height - 416) / 2
                            spacing: 28

                            Text {
                                id: txtNoLyric;
                                width: parent.width
                                anchors.left: parent.left
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                                text: qsTr("No lyrics found")
                                color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(255, 255, 255, 0.7) : Qt.rgba(0, 0, 0, 0.7)
                                font: DTK.fontManager.t5
                            }
                            Text {
                                id: txtLoadLyric;
                                width: parent.width
                                anchors.left: parent.left
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                                text: qsTr("Please put the lyric file (same name as the song) and the song file in the same folder")
                                color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(255, 255, 255, 0.7) : Qt.rgba(0, 0, 0, 0.7)
                                font: DTK.fontManager.t6
                            }
                        }
                    }
                }
            }

        }

        Rectangle {
            id: idleRect
            width: 440
            height: parent.height-164
            color: "#00000000"
            visible: titleStr.length == 0 ? true : false

            Text {
                width: parent.width
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                text: "没有找到歌词"
                color: "#70000000"
                font: DTK.fontManager.t5
            }
        }
    }
    }

    function metaChange(){
        lrcModel.clear()

        var meta = Presenter.getActivateMeta()
        titleStr = meta["title"]
        artist = meta["artist"]
        album = meta["album"]
        bgImgPath = "file:///" + meta["coverUrl"]

        if (titleStr.length === 0 && lrcRectItem.visible) {
            lyricHideAnimation.start()
            return
        }

        var lyricList = Presenter.getLyrics();
        for (var i = 0; i < lyricList.length; i++) {
            lrcModel.append(lyricList[i])
        }

        //切换shader
        switchShader();
    }

    function positionChange(position, length) {
        position = position + 500
        //二分法查找位置
        var lt,rt
        lt = 0
        rt = lrcModel.count
        while (lt < rt - 1) {
            var mid = (lt + rt) >> 1
            var item =lrcModel.get(mid)

            if (item["time"] > position)
                rt = mid
            else
                lt = mid
        }
        currentIndexChanged(lt)
    }

    function isShowShader(shaderStatus) {
        if(shaderStatus === ShaderEffect.Error) {
            shaderView.stackView.clear();
            shaderView.stackView.push(shaderView.taskMap[5]);
        }
    }

    Component.onCompleted: {
        Presenter.metaChanged.connect(metaChange)
        Presenter.positionChanged.connect(positionChange)
        shaderView.sigShaderStatusChange.connect(isShowShader)
        metaChange()
    }

    Component.onDestruction: {
        shaderView.stackView.clear();
    }

    function switchShader() {
        if(shaderView.currentItem >= 4)
            shaderView.currentItem = -1;
        shaderView.currentItem += 1;
        shaderView.stackView.clear();
        shaderView.stackView.push(shaderView.taskMap[shaderView.currentItem]);
    }

    onVisibleChanged: {
        if(visible) {
            switchShader();
        }
    }
}
