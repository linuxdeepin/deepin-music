import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: root

    property var model: artistListModel

    DGridView {
        id: artistView
        model: artistListModel
        delegate: ItemDelegate{
            cover: {
                var obj = artistView.model.get(index)
                if (obj){
                    return obj.cover
                }else{
                    return ''
                }
            }
            name: {
                var obj = artistView.model.get(index)
                if (obj){
                    return obj.name
                }else{
                    return ''
                }
            }
            count: {
                var obj = artistView.model.get(index)
                if (obj){
                    return obj.count
                }else{
                    return ''
                }
            }
        }
    }

    DListModel {
        id: artistListModel
        pymodel: ArtistListModel
    }

    ArtistController {
        artistView: artistView
    }
}