import QtQuick 2.4
import "../DMusicWidgets"

Rectangle {
    id: root

    DGridView {
        id: artistView
        model: MusicManageWorker.artists
        delegate: ItemDelegate{
            cover: GridView.view.model[index].cover
            name: GridView.view.model[index].name
            count: GridView.view.model[index].count
        }
    }

    ArtistController {
        artistView: artistView
    }
}