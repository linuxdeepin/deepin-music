import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"
import "../MusicManagerPage"

Rectangle {
    id: root

    property var model: playlistListModel
    property var view: playlistView

    height: {
        if (playlistView.count % 5 == 0){
            return  150 *  playlistView.count / 5;
        }else{
            return 150 *  (playlistView.count / 5 + 1);
        }
    }

    DGridView {
        id: playlistView
        isScrollBarVisible: false
        interactive: false
        model: playlistListModel
        delegate: PlaylistCoverItemDelegate{
            cover: {
                var obj = playlistView.model.get(index)
                if (obj){
                    return obj.cover
                }else{
                    return ''
                }
            }
            listName: {
                var obj = playlistView.model.get(index)
                if (obj){
                    return obj.listName
                }else{
                    return ''
                }
            }
            playAll: {
                var obj = playlistView.model.get(index)
                if (obj){
                    return obj.playAll
                }else{
                    return ''
                }
            }
        }
    }

    DListModel{
        id: playlistListModel
        pymodel: SuggestPlaylistListModel
    }
}