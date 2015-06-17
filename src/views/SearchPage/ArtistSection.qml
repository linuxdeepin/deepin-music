import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"
import "../MusicManagerPage"


Rectangle {
    id: root

    property var model: artistListModel
    property var view: artistView

    height: {
        if (artistView.count % 5 == 0){
            return  artistView.cellHeight *  parseInt(artistView.count / 5);
        }else{
            return artistView.cellHeight *  (parseInt(artistView.count / 5) + 1);
        }
    }

    DGridView {
        id: artistView
        isScrollBarVisible: false
        interactive: false
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
        pymodel: SearchLocalArtistListModel
    }

    ArtistController {
        artistView: artistView
    }
}