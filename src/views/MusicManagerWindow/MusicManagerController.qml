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
        MusicManageWorker.tipMessageChanged.connect(updateStatusText)
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