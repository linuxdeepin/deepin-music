import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: root

    property int topMargin: 20

    Row {

        anchors.topMargin: topMargin
        anchors.leftMargin: 14
        anchors.fill: parent

        Rectangle {
            id: leftBar
            width : 120
            height: root.height - topMargin

            PlaylistNavgationBar {
                id: playlistNavgationBar
                y: 4
                color: "white"
            }
        }


        Loader {
            id: playlistDetailLoader
            width: root.width - playlistNavgationBar.width - 14
            height: root.height - topMargin
        }
    }

    PlaylistPageController {
        playlistPage: root
        playlistNavgationBar: playlistNavgationBar
        playlistDetailLoader: playlistDetailLoader
    }
}