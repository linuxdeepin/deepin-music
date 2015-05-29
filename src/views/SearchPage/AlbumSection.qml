import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"
import "../MusicManagerPage"

Rectangle {
    id: root

    property var model: albumListModel
    property var view: albumView

    DGridView {
        id: albumView
        isScrollBarVisible: false
        interactive: false
        model: albumListModel
        delegate: SearchItemDelegate{
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
        pymodel: SearchLocalAlbumListModel
    }

    AlbumController {
        albumView: albumView
    }
}