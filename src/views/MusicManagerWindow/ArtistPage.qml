import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: root

    property var model: artistListModel

    DGridView {
        id: artistView
        model: artistListModel
        delegate: ItemDelegate{
            cover: artistView.model.get(index).cover
            name: artistView.model.get(index).name
            count: artistView.model.get(index).count
        }
    }

    DListModel {
        id: artistListModel
        pymodel: ArtistListModel
    }

    ArtistController {
        artistView: artistView
    }
}