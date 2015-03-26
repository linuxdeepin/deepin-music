import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: root

    Row {

        anchors.topMargin: 10
        anchors.leftMargin: 14
        anchors.fill: parent

        PlaylistNavgationBar {
            id: playlistNavgationBar
            width : 120
            height: root.height - 10
            color: "white"
        }

        PlaylistDetailBox {
            id: playlistDetailBox
            width: root.width - playlistNavgationBar.width
            height: root.height - 10
        }
    }

    PlaylistPageController {
        playlistNavgationBar: playlistNavgationBar
        playlistDetailBox: playlistDetailBox
    }
}