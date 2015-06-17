import QtQuick 2.3
import DMusic 1.0

Rectangle {
    id: rootBox

    property var datamodel
    property var view: listView

    Rectangle {
        id: header

        anchors.left: rootBox.left
        anchors.right: rootBox.right
        height: 24
        color: "#eeeeee"

        Row {
            anchors.fill: parent
            Rectangle{
                id: tipRect
                width: header.height
                height: 18
                color: "transparent"
            }

            Row {
                height: header.height
                spacing: 20

                Rectangle {
                    width: 266
                    height: header.height
                    color: "transparent"
                    Text {
                        id: titleText
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        color:  "#8a8a8a"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        text: I18nWorker.song + '(' + listView.count + ')'
                    }
                }

                Rectangle {
                    width: 160
                    height: header.height
                    color: "transparent"
                    Text {
                        id: artistText
                        anchors.fill: parent
                        anchors.leftMargin: 12
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
                    width: 134
                    height: header.height
                    color: "transparent"
                    Text {
                        id: sizeText
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        color:  "#8a8a8a"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        text: I18nWorker.size
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
                    width: 114
                    height: header.height
                    color: "transparent"
                    Text {
                        id: progressText
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        color:  "#8a8a8a"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        text: I18nWorker.progress
                    }
                    Rectangle {
                        x: 1
                        y: 4
                        width: 1
                        height: parent.height - 8
                        color: "lightgray"
                    }
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
        anchors.bottomMargin: 10
        ListView {
            id: listView
            anchors.fill: parent
            clip: true
            highlightMoveDuration: 1
            delegate: DownloadSongDelegate{}
            model: songListModel
            currentIndex: -1
            focus: true
            spacing: 10
            snapMode:ListView.SnapToItem
            signal playMusicByUrl(string url)
            signal switchDownloadedStatus(int songId, bool downloaded)
            signal menuShowed(int songId, bool downloaded)
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