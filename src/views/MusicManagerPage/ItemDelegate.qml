import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: item

    property var cover
    property var name
    property var count

    property var spacing: 56
    property var contentWidth: 108

    width: item.contentWidth + item.spacing
    height: item.contentWidth + 12 + 12 + 18

    Rectangle {
        anchors.fill: parent
        anchors.leftMargin: 28
        anchors.rightMargin: 28
        Column {

            ImageItem {
                source: item.cover
                width: item.contentWidth
                height: item.contentWidth
                onPlay: item.GridView.view.play(item.name, index)
                onClicked : item.GridView.view.clicked(item.name, index)
                onRightClicked: item.GridView.view.rightClicked(item.name, index)
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
                text: item.name
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
                text: item.count
            }
        }
    }
}
