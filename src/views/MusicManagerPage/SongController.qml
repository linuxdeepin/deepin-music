import QtQuick 2.3

Item {

    property var songsView

    Binding {
        target: songsView
        property: 'currentIndex'
        value: {
            var model = songsView.model
            for(var i=0; i<songsView.count; i++){
                if (model.get(i).url == MediaPlayer.url){
                    return i;
                }
            }
            return -1;
        }
    }

    Connections {
        target: songsView

        onPlayMusicByUrl: {
            SignalManager.playMusicByLocalUrl(url);
        }

        onMenuShowed:{
            MenuWorker.songMenuShow('AllSongs', url);
        }
    }

}