import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"
import "../MusicManagerPage"

Rectangle {
    id: item

    property var cover
    property var listName
    property var playAll

    property var spacing: 56
    property var contentWidth: 108

    width: item.contentWidth + item.spacing
    height: item.contentWidth + listNameText.height + 10

    Rectangle {
        anchors.fill: parent
        anchors.leftMargin: 28
        anchors.rightMargin: 28
        Column {
            id: layout
            // spacing: 10
            ImageItem {
                source: item.cover
                width: item.contentWidth
                height: item.contentWidth
                onPlay: item.GridView.view.play(item.playAll, index)
                onClicked : item.GridView.view.clicked(item.playAll, index)
                onRightClicked: item.GridView.view.rightClicked(item.playAll, index)
            }
           
            Item {
                width: item.contentWidth
                height: 10
            }

            DText {
                id: listNameText
                width: item.contentWidth
                height: 12
                color: "#252525"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                text: item.listName
            }
        }
    }
}
