import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets/MusicManager"

Rectangle {
    id: detailPage
    property var type
    property var parentItem

    anchors.fill: parent
    Column {
        anchors.fill: parent
        Rectangle {
            id: backbar
            width: detailPage.width
            height: 30
            
            DBackButton {
                id: backButton
                x: 22
                width: 30
                height: 30

                onClicked:{
                    parentItem.clearDetailLoader()
                }
            }
        }

        Rectangle {
            width: detailPage.width
            height: detailPage.height - backbar.height
            SongListView {
                id: songListView
                datamodel: DetailSongListModel
            }
        }
    }

    FolderDetailPageController {
        folderView: songListView.view
    }
}
