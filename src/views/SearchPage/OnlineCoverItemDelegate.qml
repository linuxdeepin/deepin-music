import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"
import "../MusicManagerPage"

Rectangle {
    id: item

    property var cover
    property var album
    property var artist

    property var spacing: 56
    property var contentWidth: 108

    width: item.contentWidth + item.spacing
    height: item.contentWidth + albumText.height + artistText.height + 18

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
                onPlay: item.GridView.view.play(item.album, index)
                onClicked : item.GridView.view.clicked(item.album, index)
                onRightClicked: item.GridView.view.rightClicked(item.album, index)
            }
           
            Item {
                width: item.contentWidth
                height: 10
            }

            DText {
                id: albumText
                width: item.contentWidth
                height: 12
                color: "#252525"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                text: item.album
            }
            Item {
                width: item.contentWidth
                height: 8
            }
            DText {
                id: artistText
                width: item.contentWidth
                height: 12
                color: "#888888"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                text: item.artist
            }
        }
    }
}
