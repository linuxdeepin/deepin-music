import QtQuick 2.4

Item {
    property var temporaryWindow
    property var playlistView
    property var songListModel
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

    function updateModel() {
        var pymodel = temporaryWindow.getModel()
        if (pymodel){
            songListModel.clear();
            songListModel.initModel();
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
        target: temporaryWindow
        onVisibleChanged: {
            if (visible){
                var pymodel = temporaryWindow.getModel()

                for(var i=0; i<pymodel.data.length; i++){
                    songListModel.set(i, pymodel.data[i])
                }
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

    Component.onCompleted: {
        MediaPlayer.playlistChanged.connect(updateModel);
    }
}
