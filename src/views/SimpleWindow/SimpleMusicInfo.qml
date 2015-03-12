import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: musicinfo

    property int spacing: 10
    property color spacingColor: "#2A3B4D"
    property color textColor: "white"

    property var musicImage: musicImage
    property var musicNameText: musicNameText
    property var artistNameText: artistNameText
    property var timeText: timeText
    property var musicStar: musicStar
    property var musicDownload: musicDownload

    Row {

        anchors.verticalCenter: musicinfo.verticalCenter

        spacing: musicinfo.spacing

        MusicImage {
            id: musicImage
            source: "../../skin/images/bg2.jpg"
        }

        Rectangle {

            id: msuicText
            width: musicinfo.width - musicImage.width - 2 * musicinfo.spacing
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
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    text: 'LIFE'
                }

                Text {
                    id: artistNameText
                    width: msuicText.width
                    height: 20
                    color: "white"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    text: 'Kenichior Nishihara'
                }

                Row {

                    Text {
                        id: timeText
                        width: msuicText.width - 40
                        height: 20
                        color: "white"
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        text: '02:06/04:15'
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