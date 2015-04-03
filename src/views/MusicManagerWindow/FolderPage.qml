import QtQuick 2.4
import "../DMusicWidgets"

Rectangle {
    id: root

    DGridView {
        id: folderView
        model: MusicManageWorker.folders
        delegate: ItemDelegate{
        	cover: '../../skin/images/bg4.jpg'
        	name: GridView.view.model[index].name
            count: GridView.view.model[index].count
        }
    }

    FolderController {
        folderView: folderView
    }
}