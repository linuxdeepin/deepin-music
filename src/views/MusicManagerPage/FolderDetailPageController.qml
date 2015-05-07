import QtQuick 2.3

Item {
    property var folderView
    function playMusicByUrl(url) {
        MusicManageWorker.playSong(url);
        MediaPlayer.playLocalMedia(url);
    }

    Connections {
        target: folderView

        onPlayMusicByUrl: {
            playMusicByUrl(url);
        }

        onMenuShowed:{
            MenuWorker.songMenuShow('DetailSubSongs', url);
        }
    }
}