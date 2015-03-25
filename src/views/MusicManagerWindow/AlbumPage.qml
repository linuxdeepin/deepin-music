import QtQuick 2.4
import "../DMusicWidgets"

Rectangle {
    id: root

    DGridView {
        id: grid
        model: MusicManageWorker.albums
        delegate: ItemDelegate{
        	cover: '../../skin/images/bg2.jpg'
        	name: GridView.view.model[index].name
            count: GridView.view.model[index].count
        }
    }
}