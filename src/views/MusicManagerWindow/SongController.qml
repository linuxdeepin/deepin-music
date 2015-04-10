import QtQuick 2.4

Item {
    property var titleText
    property var songsView

    function init() {
       
    }

    function playMusicByUrl(url) {
        MusicManageWorker.playSong(url);
        MediaPlayer.playLocalMedia(url);
    }

    Binding {
        target: songsView
        property: 'model'
        value: {
            print(MusicManageWorker.songs.length, '////////')
            return MusicManageWorker.songs
        }
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