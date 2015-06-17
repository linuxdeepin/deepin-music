import QtQuick 2.3
import DMusic 1.0

Rectangle {
    id: rootBox

    property var datamodel
    property var isLocal: true
    property var view: listView

    anchors.fill: parent
    anchors.leftMargin: 26
    anchors.rightMargin: 24

    Rectangle {
        id: header

        width: parent.width
        height: 24
        color: "#eeeeee"

        visible: {
            if (listView.count > 0){
                return true;
            }else{
                return false;
            }
        }

        Row {
            anchors.fill: parent
            Rectangle{
                id: tipRect
                width: 24
                height: 18
                color: "transparent"
            }

            Row {
                height: 24
                spacing: 20

                Rectangle {
                    width: 238 + 58
                    height: 24
                    color: "transparent"
                    Text {
                        id: titleText
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        color:  "#8a8a8a"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        text: I18nWorker.song + '(' + rootBox.datamodel.count + ')'
                    }
                }

                Rectangle {
                    width: 160
                    height: 24
                    color: "transparent"
                    Text {
                        id: artistText
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        color:  "#8a8a8a"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        text: I18nWorker.artist
                    }
                    Rectangle {
                        x: 1
                        y: 4
                        width: 1
                        height: parent.height - 8
                        color: "lightgray"
                    }
                }

                Rectangle {
                    width: 160
                    height: 24
                    color: "transparent"
                    Text {
                        id: albumText
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        color:  "#8a8a8a"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        text: I18nWorker.album
                    }
                    Rectangle {
                        x: 1
                        y: 4
                        width: 1
                        height: parent.height - 8
                        color: "lightgray"
                    }
                }

                Rectangle {
                    width: 116 - 58
                    height: 24
                    color: "transparent"
                    // Text {
                    //     id: durationText
                    //     anchors.fill: parent
                    //     anchors.leftMargin: 10
                    //     color:  "#8a8a8a"
                    //     font.pixelSize: 12
                    //     elide: Text.ElideRight
                    //     verticalAlignment: Text.AlignVCenter
                    //     text: I18nWorker.duration
                    // }
                    // Rectangle {
                    //     x: 1
                    //     y: 4
                    //     width: 1
                    //     height: parent.height - 8
                    //     color: "lightgray"
                    // }
                }
            }
        }
    }

    Rectangle {
        anchors.top: header.bottom
        anchors.topMargin: 1
        anchors.left: rootBox.left
        anchors.right: rootBox.right
        anchors.bottom: rootBox.bottom
        anchors.bottomMargin: 1
        ListView {
            id: listView
            anchors.fill: parent
            clip: true
            highlightMoveDuration: 1
            delegate: SongDelegate{
                isLocal: rootBox.isLocal
            }
            model: songListModel
            currentIndex: -1
            focus: true
            spacing: 10
            snapMode:ListView.SnapToItem
            signal playMusicByUrl(string url, int index, bool isDownload)
            signal menuShowed(string url, int index)

            DScrollBar {
                flickable: parent
                inactiveColor: 'black'
            }

            Component.onCompleted: positionViewAtIndex(currentIndex, ListView.Center)
        }
    }

    DListModel {
        id: songListModel
        pymodel: datamodel
    }
}