import QtQuick 2.4

Item {
    property var albumView
    property var albumListModel

    function init() {
        initArtistModel()
        MusicManageWorker.addAlbumElement.connect(albumListModel.append)
        MusicManageWorker.updateAlbumElement.connect(albumListModel.setProperty)
    }

    function initArtistModel(){
        for(var i=0; i<MusicManageWorker.albums.length; i++){
            var obj = MusicManageWorker.albums[i];
            albumListModel.append(obj);
        }
    }

    Connections {
        target: albumView
        onPlay: {
            MusicManageWorker.playAlbum(name)
        }

        onClicked:{
            print(name, 'onClicked')
        }

        onRightClicked:{
            print(name)
        }
    }

    Component.onCompleted: {
        init();
    }
}