import QtQuick 2.3

Item {
    property var playlistPage
    property var playlistNavgationBar
    property var playlistDetailBox
    property var currentPlaylistName: ''

    function init() {
        currentPlaylistName = ConfigWorker.lastPlaylistName
        if (ConfigWorker.lastPlaylistName == "favorite"){
            playlistNavgationBar.starDelegate.state = 'Checked'
        }else if(ConfigWorker.lastPlaylistName == "temporary"){
            playlistNavgationBar.temporaryDelegate.state = 'Checked'
        }else{
            for (var i = 0 ; i< PlaylistWorker.playlistNames.length; i++){
                if (PlaylistWorker.playlistNames[i].name == ConfigWorker.lastPlaylistName){
                    playlistNavgationBar.customPlaylistView.currentIndex = i;
                    return
                }
            }
        }
    }

    function playMusicByUrl(url) {
        if (url.indexOf('http') != -1){
            Web360ApiWorker.playMediaByUrl(url);
        }else{
            MediaPlayer.playLocalMedia(url);
        }
    }

    function getModelByPlaylistName(name){
        if (name){
            var model = eval('Playlist_' + Qt.md5(name));
            return model;
        }else{
            return EmptyModel
        }
    }

    Binding { 
        target: playlistNavgationBar.customPlaylistView
        property: 'model'
        value: PlaylistWorker.playlistNames
    }

    Binding {
        target: playlistDetailBox
        property: 'currentPlaylistName'
        value: currentPlaylistName
    }

    Binding {
        target: playlistDetailBox.songListModel
        property: 'pymodel'
        value: getModelByPlaylistName(currentPlaylistName)
    }

    Connections {
        target: playlistNavgationBar

        onAddPlaylistName:{
            PlaylistWorker.createPlaylistByName(name);
        }

        onPlaylistNameChanged: {
            var nameId;
            if (name == I18nWorker.favorite){
                nameId = "favorite";
                playlistNavgationBar._playlistName = ""
            }else if (name == I18nWorker.temporary){
                nameId = "temporary";
                playlistNavgationBar._playlistName = ""
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