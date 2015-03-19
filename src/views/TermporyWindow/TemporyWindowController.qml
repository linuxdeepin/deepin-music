import QtQuick 2.4

Item {
    property var playlistView
    property var clearButton
    property var closeButton


    function playMusicByUrl(url) {
        if (url.indexOf('http') != -1){
            Web360ApiWorker.playMediaByUrl(url);
        }else{
            MediaPlayer.playLocalMedia(url);
        }
    }

    function clearCurrentPlaylist() {
        MediaPlayer.playlist.clearMedias()
    }

    Binding {
        target: playlistView
        property: 'model'
        value: {
            var playlist = MediaPlayer.playlist;
            if (playlist){
                return playlist.medias;
            }else{
                return null;
            }
        }
    }

    Binding {
        target: playlistView
        property: 'currentIndex'
        value: {
            var playlist = MediaPlayer.playlist;
            if (playlist){
                return playlist.currentIndex;
            }else{
                return -1;
            }
        }
    }

    Connections {
        target: playlistView
        // onChangeIndex: changeIndex(index)
        onPlayMusicByUrl: playMusicByUrl(url)
        onModelChanged: playlistView.positionViewAtEnd()
    }

    Connections {
        target: clearButton
        onClicked: clearCurrentPlaylist() 
    }
}
