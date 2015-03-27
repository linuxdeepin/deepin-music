import QtQuick 2.4
import DMusic 1.0

Rectangle {
    id: playlistDetailBox

    property var playlistView: playlistView
    property var titleText: titleText

    Rectangle {
        id: detailBox
        anchors.fill: parent
        anchors.leftMargin: 25
        anchors.rightMargin: 35
        anchors.bottomMargin: 10

        Column {
            anchors.fill: detailBox

            spacing: 5

            Rectangle {
                id: header

                width: parent.width
                height: 24
                color: "#eeeeee"

                Row {

                    anchors.fill: parent
                    spacing: 10

                    Rectangle{
                        id: tipRect
                        width: 20
                        height: 24

                        color: "transparent"
                    }

                    Row {
                        height: 24
                        spacing: 38

                        Text {
                            id: titleText
                            width: 250
                            height: 24
                            color:  "#8a8a8a"
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            text: I18nWorker.song

                            Rectangle {
                                x: 270
                                y: 4
                                width: 1
                                height: parent.height - 8
                                color: "lightgray"
                            }
                        }

                        Text {
                            id: artistText
                            width: 156
                            height: 24
                            color: "#8a8a8a"
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            text: I18nWorker.artist

                            Rectangle {
                                x: 174
                                y: 4
                                width: 1
                                height: parent.height - 8
                                color: "lightgray"
                            }
                        }

                        Text {
                            id: durationText
                            width: 100
                            height: 24
                            color: "#8a8a8a"
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            text: I18nWorker.duration

                            
                        }
                    }
                }
            }
            Rectangle {
                width: parent.width
                height: detailBox.height - header.height - 29

                ListView {
                    id: playlistView
                    anchors.fill: parent
                    clip: true
                    highlightMoveDuration: 1
                    delegate: MusicDelegate {}
                    focus: true
                    spacing: 14
                    snapMode:ListView.SnapToItem

                    signal playMusicByUrl(string url)

                    DScrollBar {
                        flickable: parent
                        inactiveColor: 'black'
                    }
                }
            }
        }
    }
}