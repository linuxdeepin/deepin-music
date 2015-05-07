import QtQuick 2.3
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

            cover: {
                var obj = folderView.model.get(index)
                if (obj){
                    return obj.cover
                }else{
                    return ''
                }
            }
            name: {
                var obj = folderView.model.get(index)
                if (obj){
                    return UtilWorker.basename(obj.name)
                }else{
                    return ''
                }
            }
            count: {
                var obj = folderView.model.get(index)
                if (obj){
                    return obj.count
                }else{
                    return ''
                }
            }

        }
    }

    DListModel {
        id: folderListModel
        pymodel: FolderListModel
    }

    FolderController {
        folderView: folderView
        folderListModel: folderListModel
    }
}