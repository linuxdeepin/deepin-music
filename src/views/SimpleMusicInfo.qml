import QtQuick 2.0
import DMusic 1.0
import "./dmusicwidgets"

Rectangle {
    id: musicinfo

    property int spacing: 10
    property color spacingColor: "#2A3B4D"
    property color textColor: "white"

    Row {

        anchors.verticalCenter: musicinfo.verticalCenter

        spacing: musicinfo.spacing

        Rectangle{
            id: musicImage
            width: 60
            height: 60

            BorderImage {
                anchors.fill: musicImage
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode: BorderImage.Stretch
                source: "../skin/images/bg2.jpg"
            }
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
                    width: msuicText.width
                    height: 20
                    color: "white"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    text: 'LIFE'
                }

                Text {
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
                        id: musicstar
                        width: 20
                        height: 20
                    }

                    DDownloadButton{
                        id: musicdownload
                        width: 20
                        height: 20
                    }
                }
            }
        }
    }

}