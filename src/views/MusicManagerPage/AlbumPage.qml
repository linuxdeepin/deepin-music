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
            cover: {
                var obj = albumView.model.get(index)
                if (obj){
                    return obj.cover
                }else{
                    return ''
                }
            }
            name: {
                var obj = albumView.model.get(index)
                if (obj){
                    return obj.name
                }else{
                    return ''
                }
            }
            count: {
                var obj = albumView.model.get(index)
                if (obj){
                    return obj.count
                }else{
                    return ''
                }
            }
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