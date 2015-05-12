import QtQuick 2.3

Item {
    property var playlistPage
    property var playlistNavgationBar
    property var playlistDetailLoader
    property var currentPlaylistName: ''

    function init() {
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
        currentPlaylistName = ConfigWorker.lastPlaylistName

        MediaPlayer.currentIndexChanged.connect(activeCurrentItem)
        MediaPlayer.playingChanged.connect(activeCurrentItem)
    }

    function playMusicByUrl(url) {
        if (url.indexOf('http') != -1){
            Web360ApiWorker.playMediaByUrl(url);
        }else{
            MediaPlayer.playLocalMedia(url);
        }
    }

    function activeCurrentItem(currentIndex){
        var playlistDetailBox = playlistDetailLoader.item
        if (MediaPlayer.playlist && playlistDetailBox) {
            if (MediaPlayer.playlist.name == currentPlaylistName){
                playlistDetailBox.playlistView.currentIndex = MediaPlayer.playlist.currentIndex;
                if (playlistDetailBox.playlistView.currentItem){
                    if (MediaPlayer.playing){
                        playlistDetailBox.playlistView.currentItem.state = "Active";
                    }else{
                        playlistDetailBox.playlistView.currentItem.state = "!Checked";
                    }
                }
            }

            if (MediaPlayer.playing){
                if (MediaPlayer.playlist.name == 'favorite'){
                    playlistNavgationBar.starDelegate.state = 'Active'
                    if (currentPlaylistName == 'temporary'){
                        playlistNavgationBar.temporaryDelegate.state = 'Checked'
                    }else{
                        playlistNavgationBar.temporaryDelegate.state = '!Checked'
                    }

                }else if(MediaPlayer.playlist.name == 'temporary'){
                    playlistNavgationBar.temporaryDelegate.state = 'Active'
                    if (currentPlaylistName == 'favorite'){
                        playlistNavgationBar.starDelegate.state = 'Checked'
                    }else{
                        playlistNavgationBar.starDelegate.state = '!Checked'
                    }
                }else{

                }
            }else{
               if (currentPlaylistName == 'favorite'){
                    playlistNavgationBar.starDelegate.state = 'Checked'
                    playlistNavgationBar.temporaryDelegate.state = '!Checked'

                }else if(currentPlaylistName == 'temporary'){
                    playlistNavgationBar.starDelegate.state = '!Checked'
                    playlistNavgationBar.temporaryDelegate.state = 'Checked'
                }else{

                }
            }
        }
    }

    function getModel(){
        if (currentPlaylistName){
            var model = eval('Playlist_' + Qt.md5(currentPlaylistName));
            return model;
        }
    }

    Binding { 
        target: playlistNavgationBar.customPlaylistView
        property: 'model'
        value: PlaylistWorker.playlistNames
    }

    Connections {
        target: playlistPage
        onVisibleChanged: {
            if (visible){
                var playlist = MediaPlayer.playlist;
                if (playlist){
                    var name = playlist.name;
                    playlistNavgationBar.playlistNameChanged(name);
                }
            }
        } 
    }

    Connections {
        target: playlistNavgationBar

        onAddPlaylistName:{
            PlaylistWorker.createPlaylistByName(name);
            if (playlistNavgationBar.starDelegate.state == "Active"){
                
            }else{
                playlistNavgationBar.starDelegate.state = '!Checked'
            }
            if (playlistNavgationBar.temporaryDelegate.state == "Active"){
                
            }else {
                playlistNavgationBar.temporaryDelegate.state = '!Checked'
            }
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

            // var pymodel = getModel()

            playlistDetailLoader.setSource('./PlaylistDetailBox.qml', {'currentPlaylistName': currentPlaylistName})

            activeCurrentItem();

            if (playlistDetailLoader.item.playlistView.count == 0){
                playlistDetailLoader.item.noMusicTip.visible = true;
            }else{
                playlistDetailLoader.item.noMusicTip.visible = false;
            }
        }
    }

    Component.onCompleted: {
        init()
    }
}