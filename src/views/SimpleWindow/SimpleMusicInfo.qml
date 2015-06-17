import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"
import "../DMusicWidgets/SimplePlayer"

Rectangle {
    id: musicInfo

    property int spacing: 10
    property color spacingColor: "#2A3B4D"
    property color textColor: "white"

    property var musicImage: musicImage
    property var musicNameText: musicNameText
    property var artistNameText: artistNameText
    property var timeText: timeText
    property var musicStar: musicStar
    property var musicDownload: musicDownload

    property var title
    property var artist
    property var cover

    Row {

        anchors.verticalCenter: musicInfo.verticalCenter

        spacing: musicInfo.spacing

        MusicImage {
            id: musicImage
            source: musicInfo.cover
        }

        Rectangle {

            id: msuicText
            width: musicInfo.width - musicImage.width - 2 * musicInfo.spacing
            height: 60
            color: 'transparent'

            Column {

                anchors.centerIn: parent
                anchors.margins: 20

                Text {
                    id: musicNameText
                    width: msuicText.width
                    height: 20
                    color: "white"
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    text: musicInfo.title
                }

                Text {
                    id: artistNameText
                    width: msuicText.width
                    height: 20
                    color: "white"
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    text: musicInfo.artist
                }

                Row {

                    Text {
                        id: timeText
                        width: msuicText.width - 40
                        height: 20
                        color: "white"
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        text: ''
                    }

                    DStarButton{
                        id: musicStar
                        width: 20
                        height: 20
                    }

                    DDownloadButton{
                        id: musicDownload
                        width: 20
                        height: 20
                    }
                }
            }
        }
    }
}