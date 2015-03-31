import QtQuick 2.4

Item {
    property var rootWindow
    property var catgoryCombox
    property var statusText
    property var musicManagerLoader
    property var noMusicTip
    property var linkTipText


    function initConnect() {
        MusicManageWorker.scanfileChanged.connect(updateStatusText)
    }

    function updateWindow(index) {
        if (index == 0){
            musicManagerLoader.source = './ArtistPage.qml';
        }else if(index == 1){
            musicManagerLoader.source = './AlbumPage.qml';
        }else if (index == 2){
            musicManagerLoader.source = './SongPage.qml';
        }else if (index == 3){
            musicManagerLoader.source = './FolderPage.qml';
        }
    }

    function updateStatusText(text){
        statusText.text = text;
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
        target: rootWindow
        onVisibleChanged:{
            catgoryCombox.visible = false;
            if (visible){
                updateWindow(catgoryCombox.currentIndex);
            }
            if (MusicManageWorker.songCount == 0){
                noMusicTip.visible = true;
            }
        }
    }

    Connections {
        target: linkTipText
        onLinkActivated: {
            if (link == "SearchAllDriver"){
                MusicManageWorker.searchAllDriver();
                noMusicTip.visible = false
                updateWindow(3)
                catgoryCombox.currentText = catgoryCombox.view.model[3].name
            }else if (link == "SearchOneFolder"){
                MusicManageWorker.searchOneFolder();
            }
        }
    }

    Component.onCompleted: {
        initConnect();
    }
}