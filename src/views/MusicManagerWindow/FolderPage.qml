import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: root

    DGridView {
        id: folderView
        anchors.leftMargin: 20
        anchors.rightMargin: 40
        cellWidth: 64 + 88
        cellHeight: 64 + 12 + 12 + 30 + 10
        model: folderListModel
        delegate: ItemDelegate{
            spacing: 96
            contentWidth: 64
            height: 64 + 12 + 12 + 30
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