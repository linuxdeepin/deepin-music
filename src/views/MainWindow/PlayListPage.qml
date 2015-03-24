import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: root

    Row {

        anchors.topMargin: 10
        anchors.leftMargin: 14
        anchors.fill: parent

        Rectangle {
            id: playlistNameBar
            width : 120
            height: root.height
            color: "white"

            Column {

                
                anchors.fill: parent

                Rectangle {
                    id: fixPlaylistBar
                    width: 120
                    height: 64

                    Column {
                        spacing: 16

                        Row {
                            spacing: 10
                            DStarButton {
                                id: starButton
                                width: 16
                                height: 16
                            }

                            Text {
                                width: fixPlaylistBar.width - starButton.width
                                height: 16
                                color: "#868686"
                                font.pixelSize: 12
                                verticalAlignment: Text.AlignVCenter
                                text: '我的收藏'
                            }
                        }

                        Row {
                            spacing: 10

                            DStarButton {
                                id: wavButton
                                width: 16
                                height: 16
                            }

                            Text {
                                width: fixPlaylistBar.width - wavButton.width
                                height: 16
                                color: "#868686"
                                font.pixelSize: 12
                                verticalAlignment: Text.AlignVCenter
                                text: '试听歌单'
                            }

                        }
                    }

                }

                Rectangle {
                    id: separator
                    width: 120
                    height: 1
                    color: "#dfdfdf"
                }

                Rectangle {
                    id: playlistBox
                    width: 120
                    height: playlistNameBar.height - separator.height - fixPlaylistBar.height - 10
                    color: "white"

                    Rectangle {
                        id: playlistTitle
                        width: parent.width
                        height: 42
                        Rectangle {
                            anchors.fill: parent
                            anchors.topMargin: 14
                            anchors.bottomMargin: 12
                            width: parent.width
                            height: 16
                            Text {
                                anchors.centerIn: parent
                                width: 100
                                height: 11
                                color: "#b9b9b9"
                                font.pixelSize: 11
                                text: '我创建的歌单'
                            }

                            DAddButton {
                                anchors.right: parent.right
                                width: 16
                                height: 16
                            }
                        }
                    }

                    Component {
                        id: playlistDeletgate

                        Rectangle{
                            width: playlistBox.width
                            height: 16
                            Row {
                                spacing: 10

                                DPlaylistButton {
                                    width: 16
                                    height: 16
                                }
                                Text {
                                    width: 94
                                    height: 16
                                    color: "#868686"
                                    font.pixelSize: 12
                                    text: '12222222'
                                }
                            }
                        }
                    }

                    ListView {
                        clip: true
                        anchors.top: playlistTitle.bottom
                        anchors.bottom: playlistBox.bottom
                        width: 120
                        highlightMoveDuration: 1
                        model: 10
                        delegate: playlistDeletgate
                        focus: true
                        spacing: 14
                        // displayMarginBeginning: -24
                        // displayMarginEnd: -24
                        snapMode:ListView.SnapToItem

                        // signal playMusicByUrl(string url)

                        DScrollBar {
                            flickable: parent
                            inactiveColor: 'black'
                        }
                    }
                }
                
            }
        }

        Rectangle {
            width: root.width - playlistNameBar.width
            height: root.height
        }

    }
}