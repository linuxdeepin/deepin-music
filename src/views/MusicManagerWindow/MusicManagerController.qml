import QtQuick 2.4

Item {
    property var rootWindow
    property var catgoryCombox
    property var statusText
    property var scanStatusText
    property var musicManagerLoader
    property var noMusicTip
    property var linkTipText


    function initConnect() {
        MusicManageWorker.songCountChanged.connect(showArtistPage);
        MusicManageWorker.tipMessageChanged.connect(updateStatusText);
    }

    function showArtistPage(){
        if (MusicManageWorker.songCount == 0){
            linkTipText.visible = true;
        }else{
            linkTipText.visible = false;
            updateWindow(catgoryCombox.currentIndex);
        }
    }

    function updateWindow(index) {
        if (index == 0){
            musicManagerLoader.source = './ArtistPage.qml';
            WindowManageWorker.currentMusicManagerPageName = 'ArtistPage'
        }else if(index == 1){
            musicManagerLoader.source = './AlbumPage.qml';
            WindowManageWorker.currentMusicManagerPageName = 'AlbumPage'
        }else if (index == 2){
            musicManagerLoader.source = './SongPage.qml';
            WindowManageWorker.currentMusicManagerPageName = 'SongPage'
        }else if (index == 3){
            musicManagerLoader.source = './FolderPage.qml';
            WindowManageWorker.currentMusicManagerPageName = 'FolderPage'
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

    // Binding {
    //     target: rootWindow
    //     property: 'visible'
    //     value: MusicManageWorker.songCount
    // }

    Connections {
        target: rootWindow
        onVisibleChanged: {
            if (visible == true){
                if (MusicManageWorker.songCount == 0){
                    print('+++++++++', MusicManageWorker.songCount)
                    noMusicTip.visible = true;
                }else{
                    noMusicTip.visible = false;
                }
            }
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