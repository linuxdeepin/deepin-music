import QtQuick 2.3

Item {
    property var allDownloadButton
    property var allPausedButton
    property var openFolderButton
    property var songListView

    function init(){
        MenuWorker.switchDownloadedStatus.connect(songListView.view.switchDownloadedStatus)
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

    Connections {
        target: songListView.view

        onPlayMusicByUrl:{
            Web360ApiWorker.playMediaByUrl(url);
        }

        onSwitchDownloadedStatus: {
            if (downloaded){
                DownloadSongWorker.oneStartDownloadSignal(songId);
            }else{
                DownloadSongWorker.onePausedDownloadSignal(songId);
            }
        }

        onMenuShowed:{
            MenuWorker.downloadMenuShowed(songId, downloaded);
        }
    }

    Component.onCompleted: {
        init();
    }
}