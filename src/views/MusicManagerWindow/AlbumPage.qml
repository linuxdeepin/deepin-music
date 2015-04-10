import QtQuick 2.4
import "../DMusicWidgets"

Rectangle {
    id: root

    DGridView {
        id: albumView
        model: MusicManageWorker.albums
        delegate: ItemDelegate{
        	cover: GridView.view.model[index].cover
        	name: GridView.view.model[index].name
            count: GridView.view.model[index].count
        }
    }

    ListModel{
        id: albumListModel
    }

    AlbumController {
        albumView: albumView
        albumListModel: albumListModel
    }
}