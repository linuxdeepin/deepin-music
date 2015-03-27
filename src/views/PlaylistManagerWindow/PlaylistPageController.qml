import QtQuick 2.4

Item {

    property var playlistNavgationBar
    property var playlistDetailBox
    property var currentPlaylistName: ''

    function init() {
        if (ConfigWorker.lastPlaylistName == "favorite"){
            playlistNavgationBar.starList.state = 'Checked'
        }else if(ConfigWorker.lastPlaylistName == "temporary"){
            playlistNavgationBar.temporaryList.state = 'Checked'
        }else{
            for (var i = 0 ; i< PlaylistWorker.playlistNames.length; i++){
                if (PlaylistWorker.playlistNames[i].name == ConfigWorker.lastPlaylistName){
                    playlistNavgationBar.customPlaylistView.currentIndex = i;
                    return
                }
            }
        }
        currentPlaylistName = ConfigWorker.lastPlaylistName
    }

    Binding { 
        target: playlistNavgationBar.customPlaylistView
        property: 'model'
        value: PlaylistWorker.playlistNames
    }

    Binding {
        target: playlistDetailBox.playlistView
        property: 'model'
        value:{
            if (currentPlaylistName) {
                var medias = PlaylistWorker.getMediasByName(currentPlaylistName);
                return medias
            }
        }
    }

    Connections {
        target: playlistDetailBox.playlistView
        onModelChanged: {
            playlistDetailBox.playlistView.currentIndex = -1;
        } 
    }

    Connections {
        target: playlistNavgationBar

        onAddPlaylistName:{
            PlaylistWorker.createPlaylistByName(name);
            playlistNavgationBar.starList.state = '!Active'
            playlistNavgationBar.temporaryList.state = '!Active'
        }

        onPlaylistNameChanged: {
            var nameId;
            if (name == I18nWorker.favorite){
                nameId = "favorite";
            }else if (name == I18nWorker.temporary){
                nameId = "temporary";
            }else{
                nameId = name;
            }
            currentPlaylistName = nameId;
        }
    }

    Component.onCompleted: {
        init()
    }
}