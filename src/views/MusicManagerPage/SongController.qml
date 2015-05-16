import QtQuick 2.3

Item {

    property var songsView

    function playMusicByUrl(url) {
        MusicManageWorker.playSong(url);
        MediaPlayer.playLocalMedia(url);
    }

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
            playMusicByUrl(url);
        }

        onMenuShowed:{
            MenuWorker.songMenuShow('AllSongs', url);
        }
    }

}