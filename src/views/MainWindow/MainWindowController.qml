import QtQuick 2.3

Item {
    property var mainWindow
    property var bgImage
	property var titleBar
	property var leftSideBar
	property var webEngineViewPage
    property var playBottomBar
    property var dSimpleWindow


    function initConnect(){
        WindowManageWorker.switchPageByID.connect(leftSideBar.swicthViewByID);
    }

    function resetSkin() {
        playBottomBar.color = "#282F3F"
        bgImage.source = ''
    }


    function setSkinByImage(url) {
        if (url){
            bgImage.source = url
        }else{
            resetSkin();
        }
    }

    Connections {
        target: bgImage
        onProgressChanged:{
            if (progress == 1){
                playBottomBar.color = "transparent"
            }
        }
    }

    Connections {
        target: leftSideBar
        onSwicthViewByID: {
            var index =  mainWindow.views.indexOf(viewID);
            stackViews.setCurrentIndex(index);
        }
    }

    Connections {
        target: webEngineViewPage
        onPlayMusicById: Web360ApiWorker.playMusicByIdSignal(musicId)
        onPlayMusicByIds: Web360ApiWorker.playMusicByIdsSignal(musicIds)
        onPlaySonglistById: Web360ApiWorker.playSonglistByIdSignal(songlistId)
        onPlaySonglistByName: Web360ApiWorker.playSonglistByNameSignal(songlistName)
        onPlayAlbumById: Web360ApiWorker.playAlbumByIdSignal(albumId)

        onAddFavorite: Web360ApiWorker.addFavoriteSignal(musicId)
        onRemoveFavorite: Web360ApiWorker.removeFavoriteSignal(musicId)

        onDownloadSong: Web360ApiWorker.downloadSongSignal(musicId)
        onDownloadSongs: Web360ApiWorker.downloadSongsSignal(musicIds)
    }

    Component.onCompleted: {
        initConnect();
    }
}
