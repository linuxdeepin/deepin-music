import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"
import "../MusicManagerPage"

Rectangle {
    id: root

    property var model: albumListModel
    property var view: albumView

    height: {
        if (albumView.count % 5 == 0){
            return  albumView.cellHeight *  parseInt(albumView.count) / 5;
        }else{
            return albumView.cellHeight *  (parseInt(albumView.count / 5) + 1);
        }
    }

    DGridView {
        id: albumView
        isScrollBarVisible: false
        // interactive: false
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