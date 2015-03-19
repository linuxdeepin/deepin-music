import QtQuick 2.4

Item {
    property var playlistView
    property var clearButton
    property var closeButton

    function changeIndex(index) {
        MediaPlayer.setCurrentMedia(index);
        MediaPlayer.playToggle(true);
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
        onChangeIndex: changeIndex(index)
        onModelChanged: playlistView.positionViewAtEnd()
    }

    Connections {
        target: clearButton
        onClicked: clearCurrentPlaylist() 
    }
}
