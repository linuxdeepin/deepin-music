import QtQuick 2.4
import "../DMusicWidgets"

Rectangle {
    id: root

    DGridView {
        id: grid
        model: MusicManageWorker.artists
        delegate: ItemDelegate{
            cover: '../../skin/images/bg1.jpg'
            name: GridView.view.model[index].name
            count: GridView.view.model[index].count
        }
    }
}