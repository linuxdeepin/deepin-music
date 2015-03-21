import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: item

    property var contentWidth: 108

    width: item.contentWidth + 56
    height: item.contentWidth + 12 + 12 + 18

    Rectangle {
        anchors.fill: parent
        anchors.leftMargin: 28
        anchors.rightMargin: 28
        Column {
            Image {
                width: item.contentWidth
                height: item.contentWidth
                source: '../../skin/images/bg1.jpg'
            }
            Item {
                width: item.contentWidth
                height: 10
            }

            DText {
                width: item.contentWidth
                height: 12
                color: "#252525"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                text: 'liudehua'
            }
            Item {
                width: item.contentWidth
                height: 8
            }
            DText {
                width: item.contentWidth
                height: 12
                color: "#888888"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                text: '4'
            }
        }
    }
}
