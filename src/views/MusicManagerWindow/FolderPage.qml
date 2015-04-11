import QtQuick 2.4
import DMusic 1.0
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

    DListModel {
        id: folderListModel
        pymodel: FolderListModel
    }

    FolderController {
        folderView: folderView
    }
}