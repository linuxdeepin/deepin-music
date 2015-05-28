import QtQuick 2.3

Item {

    property var localSongsView

    function playMusicByUrl(url) {
        MusicManageWorker.playSong(url);
        MediaPlayer.playLocalMedia(url);
    }

    Binding {
        target: localSongsView
        property: 'currentIndex'
        value: {
            var model = localSongsView.model
            for(var i=0; i<localSongsView.count; i++){
                if (model.get(i).url == MediaPlayer.url){
                    return i;
                }
            }
            return -1;
        }
    }

    Connections {
        target: localSongsView

        onPlayMusicByUrl: {
            playMusicByUrl(url);
        }

        onMenuShowed:{
            MenuWorker.songMenuShow('AllSongs', url);
        }
    }

}