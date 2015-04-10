import QtQuick 2.4
import "../DMusicWidgets"

Rectangle {
    id: root

    DGridView {
        id: folderView
        model: folderListModel
        delegate: ItemDelegate{
            cover: folderView.model.get(index).cover
            name: folderView.model.get(index).name
            count: folderView.model.get(index).count
        }
    }

    ListModel {
        id: folderListModel
    }

    FolderController {
        folderView: folderView
        folderListModel: folderListModel
    }
}