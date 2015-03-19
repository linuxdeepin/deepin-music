import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

DRectWithCorner {
    id: termporyWindow

    property var playlistView: playlistView
    property var clearButton: clearButton
    property var closeButton: closeButton
    property var models
    property var currentIndex: -1

    rectWidth: 302 + 4
    rectHeight: 402 + 12 + 20
    rectRadius: 0
    blurRadius: 4
    blurWidth: 4
    cornerPos: 260
    cornerWidth: 20
    cornerHeight: 10
    color: 'transparent'


    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: false
        acceptedButtons: Qt.LeftButton

        onWheel: {
            wheel.accepted = true;
        }
    }

    Component {
        id: musicDelegate
        MusicDelegate {}
    }

    Column{
        id: layout
        x: 4
        y: 4
        height: 402
        width: 302

        Rectangle{
            id: titleBar
            width: layout.width
            height: 34
            color: "white"

            Row {
                anchors.centerIn: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14

                Text{
                    id: playlistName
                    width: titleBar.width - 28 - clearButton.width - closeButton.width
                    height: 25
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 16
                    color: "#676767"
                    text: '正在播放'

                    Text {
                        x: playlistName.contentWidth
                        width: 40
                        height: 25
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 16
                        color: "#a0a0a0"
                        text: {
                            var playlist = MediaPlayer.playlist;
                            if (playlist){
                                return '(' + playlist.count + ')';
                            }else{
                                return '';
                            }
                        }
                    }
                }

                DDeleteButton {
                    id: clearButton
                    width: 25
                    height: 25
                }

                DCloseButton {
                    id: closeButton
                    width: 25
                    height: 25
                }

            }
        }

        Rectangle {
            id: separator
            height: 1
            width: layout.width
            color: "transparent"
            Rectangle{
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                color: "lightgray"
            }
        }

        Rectangle {
            id: playlistContainer
            height: 402 - titleBar.height - separator.height
            width: layout.width

            ListView {
                id: playlistView
                anchors.fill: playlistContainer
                topMargin: 10
                bottomMargin: 10
                highlightMoveDuration: 1
                delegate: musicDelegate
                focus: true
                spacing: 8
                displayMarginBeginning: -24
                displayMarginEnd: -24
                snapMode:ListView.SnapToItem

                signal changeIndex(int index)
                signal playMusicByUrl(string url)

                DScrollBar {
                    flickable: parent
                    inactiveColor: 'black'
                }

                Component.onCompleted: positionViewAtIndex(currentIndex, ListView.Center)
            }
        }
    }

    TemporyWindowController {
        id: temporyWindowController
        playlistView: playlistView
        clearButton: clearButton
        closeButton: closeButton
    }
}