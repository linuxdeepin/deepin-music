import QtQuick 2.3
import DMusic 1.0

Rectangle {
    id: root
    anchors.fill: parent
    Rectangle {
        id: contentBox
        anchors.fill: parent
        anchors.leftMargin: 34
        anchors.rightMargin: 34
        color: "white"
        Column {
            id: layout

            anchors.fill: parent
            spacing: 14

            Rectangle {
                id: tootBar
                width: contentBox.width
                height: 20
            }

            DownloadSongListView {
                id: songListView
                datamodel: DownloadSongListModel
                width: contentBox.width
                height: contentBox.height - layout.spacing - tootBar.height
            }
        }
    }
}