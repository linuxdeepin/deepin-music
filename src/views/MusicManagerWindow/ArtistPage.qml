import QtQuick 2.4
import "../DMusicWidgets"

Rectangle {
    id: root

    DGridView {
        id: artistView
        delegate: ItemDelegate{
            cover: artistView.model.get(index).cover
            name: artistView.model.get(index).name
            count: artistView.model.get(index).count
        }
    }

    ListModel {
        id: artistListModel
    }

    ArtistController {
        artistView: artistView
        artistListModel: artistListModel
    }
}