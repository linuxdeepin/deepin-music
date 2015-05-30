import QtQuick 2.3

Item {
    property var detailPage
    property var detailImageItem
    property var detailView
    property var songListModel

    Binding {
        target: detailView
        property: 'currentIndex'
        value: {
            var model = detailView.model
            for(var i=0; i<detailView.count; i++){
                if (model.get(i).url == MediaPlayer.url){
                    return i;
                }
            }
        }
    }

    Connections {
        target: detailView

        onPlayMusicByUrl: {
            SignalManager.playMusicByLocalUrl(url);
        }

        onMenuShowed:{
            MenuWorker.songMenuShow('DetailSubSongs', url);
        }
    }

    Connections {
        target: detailImageItem
        onPlay: {
            if(detailPage.type == 'Artist'){
                var name = detailPage.model.name;
                MusicManageWorker.playArtist(name);
            }
            else if (detailPage.type == 'Album'){
                var name = detailPage.model.name;
                MusicManageWorker.playAlbum(name);
            }
        } 
    }
}