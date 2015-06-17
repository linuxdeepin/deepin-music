import QtQuick 2.3

Item {
    property var rootWindow
    property var catgoryCombox
    property var statusText
    property var scanStatusText
    property var musicManagerLoader
    property var detailLoader
    property var noMusicTip
    property var linkTipText


    function initConnect() {
        MusicManageWorker.tipMessageChanged.connect(updateStatusText);
        MusicManageWorker.detailArtist.connect(showDetailArtist)
        MusicManageWorker.detailAlbum.connect(showDetailAlbum)
        MusicManageWorker.detailFolder.connect(showDetailFolder)
    }

    function showDetailArtist(name, index) {
        MusicManageWorker.updateDetailSongObjsByArtist(name)
        detailLoader.setSource('./DetailPage.qml', { 
            'model': musicManagerLoader.item.model.get(index),
            'type': 'Artist',
            'parentItem':rootWindow
        })
    }

    function showDetailAlbum(name, index) {
        MusicManageWorker.updateDetailSongObjsByAlbum(name)
        detailLoader.setSource('./DetailPage.qml', { 
            'model': musicManagerLoader.item.model.get(index),
            'type': 'Album',
            'parentItem': rootWindow})
    }

    function showDetailFolder(name, index) {
        var folderName = FolderListModel.data[index].name;
        MusicManageWorker.updateDetailSongObjsByFolder(folderName);
        detailLoader.setSource('./FolderDetailPage.qml', {
            'type': 'Folder',
            'parentItem': rootWindow})
    }

    function updateWindow(index) {
        if (index == 0){
            musicManagerLoader.source = './ArtistPage.qml';
            WindowManageWorker.currentMusicManagerPageName = 'ArtistPage'
            MusicManageWorker.switchPage('ArtistPage')
        }else if(index == 1){
            musicManagerLoader.source = './AlbumPage.qml';
            WindowManageWorker.currentMusicManagerPageName = 'AlbumPage'
            MusicManageWorker.switchPage('AlbumPage')
        }else if (index == 2){
            musicManagerLoader.source = './SongPage.qml';
            WindowManageWorker.currentMusicManagerPageName = 'SongPage'
            // MusicManageWorker.switchPage('SongPage')
        }else if (index == 3){
            musicManagerLoader.source = './FolderPage.qml';
            WindowManageWorker.currentMusicManagerPageName = 'FolderPage'
            MusicManageWorker.switchPage('FolderPage')
        }
    }

    function updateStatusText(text){
        scanStatusText.text = text;
    }

    Binding {
        target: noMusicTip
        property: 'visible'
        value: {
            if (MusicManageWorker.songCount == 0){
                return true;
            }else{
                return false
            }
        }
    }

    Connections {
        target: rootWindow
        onVisibleChanged: {
            if (visible == true){
                if (MusicManageWorker.songCount == 0){
                    noMusicTip.visible = true;
                }else{
                    noMusicTip.visible = false;
                }
            }
        }

        onClearDetailLoader:{
            detailLoader.setSource('')
        }
    }

    Connections {
        target: catgoryCombox
        onCurrentIndexChanged: {
            var currentIndex = catgoryCombox.currentIndex;
            updateWindow(currentIndex);
        }

        onItemClicked: {
            var currentIndex = catgoryCombox.currentIndex;
            catgoryCombox.visible = false
        }
    }

    Connections {
        target: musicManagerLoader
        onLoaded: {
            musicManagerLoader.focus = true;
        } 
    }

    Connections {
        target: linkTipText
        onLinkActivated: {
            if (link == "SearchAllDriver"){
                MusicManageWorker.searchAllDriver();
            }else if (link == "SearchOneFolder"){
                MusicManageWorker.searchOneFolder();
            }
            updateWindow(0)
            catgoryCombox.currentText = catgoryCombox.view.model[0].name
        }
    }

    Component.onCompleted: {
        initConnect();
    }
}