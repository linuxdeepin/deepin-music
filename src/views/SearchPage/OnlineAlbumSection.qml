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
        delegate: OnlineCoverItemDelegate{
            cover: {
                var obj = albumView.model.get(index)
                if (obj){
                    return obj.cover
                }else{
                    return ''
                }
            }
            album: {
                var obj = albumView.model.get(index)
                if (obj){
                    return obj.album
                }else{
                    return ''
                }
            }
            artist: {
                var obj = albumView.model.get(index)
                if (obj){
                    return obj.artist
                }else{
                    return ''
                }
            }
        }
    }

    DListModel{
        id: albumListModel
        pymodel: SearchOnlineAlbumListModel
    }
}