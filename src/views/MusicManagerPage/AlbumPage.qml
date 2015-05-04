import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: root

    property var model: albumListModel

    DGridView {
        id: albumView
        model: albumListModel
        delegate: ItemDelegate{
        	cover: GridView.view.model.get(index).cover
        	name: GridView.view.model.get(index).name
            count: GridView.view.model.get(index).count
        }
    }

    DListModel{
        id: albumListModel
        pymodel: AlbumListModel
    }

    AlbumController {
        albumView: albumView
    }
}