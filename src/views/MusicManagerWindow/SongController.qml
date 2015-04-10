import QtQuick 2.4

Item {
    property var titleText
    property var songsView
    property var songListModel

    function init() {
       initSongListModel();
       MusicManageWorker.addSongElement.connect(songListModel.append)
    }

    function initSongListModel(){
        var songs = MusicManageWorker.songs
        for(var i=0; i<songs.length; i++){
            var obj = songs[i];
            songListModel.append(obj);
        }
        
    }

    function playMusicByUrl(url) {
        MusicManageWorker.playSong(url);
        MediaPlayer.playLocalMedia(url);
    }

    Binding {
        target: songsView
        property: 'model'
        value: songListModel
    }

    Connections {
        target: songsView

        onPlayMusicByUrl: {
            playMusicByUrl(url);
        }
    }

    Component.onCompleted: {
        init()
    }
}