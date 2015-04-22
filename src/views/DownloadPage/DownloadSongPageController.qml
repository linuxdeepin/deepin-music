import QtQuick 2.3

Item {
    property var allDownloadButton
    property var allPausedButton
    property var openFolderButton
    property var songListView

    function playMusicByUrl(url) {
        MusicManageWorker.playSong(url);
        MediaPlayer.playLocalMedia(url);
    }

    Connections {
        target: allDownloadButton
        onClicked: {
            DownloadSongWorker.allStartDownloadSignal();
        } 
    }

    Connections {
        target: allPausedButton
        onClicked: {
            DownloadSongWorker.allPausedSignal();
        }
    }

    Connections {
        target: openFolderButton
        onClicked: {
            UtilWorker.openUrl(ConfigWorker.DownloadSongPath)
        } 
    }
}