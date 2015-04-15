import QtQuick 2.4

Item {
    property var playlistPage
    property var playlistNavgationBar
    property var playlistDetailBox
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
        if (MediaPlayer.playlist) {
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
            var model = eval(currentPlaylistName);
            return model;
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
        target: playlistDetailBox.titleText
        property: 'text'
        value: {
            if (playlistDetailBox.playlistView.model) {
                var model = playlistDetailBox.playlistView.model;
                return I18nWorker.song + '   (' + model.length +')'
            }
        }
    }

    Connections {
        target: playlistDetailBox.playlistView

        onPlayMusicByUrl: {
            MediaPlayer.setPlaylistByName(currentPlaylistName);
            playMusicByUrl(url);

            if (MediaPlayer.playlist.name == 'favorite'){
                playlistNavgationBar.starDelegate.state = 'Active'
                playlistNavgationBar.temporaryDelegate.state = '!Checked'

            }else if(MediaPlayer.playlist.name == 'temporary'){
                playlistNavgationBar.starDelegate.state = '!Checked'
                playlistNavgationBar.temporaryDelegate.state = 'Active'
            }else{

            }
        }

        onModelChanged: {
            playlistDetailBox.playlistView.positionViewAtEnd()
            playlistDetailBox.playlistView.currentIndex = -1;
            activeCurrentItem();
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

            print(getModel())
            playlistDetailBox.songListModel.pymodel = getModel()

            activeCurrentItem();

            if (playlistDetailBox.playlistView.count == 0){
                playlistDetailBox.noMusicTip.visible = true;
            }else{
                playlistDetailBox.noMusicTip.visible = false;
            }
        }
    }

    Connections {
        target: playlistDetailBox.linkTipText
        onLinkActivated: {
            if (link == "Online"){
                WindowManageWorker.switchPageByID('WebMusic360Page');
            }else if (link == "Local"){
                WindowManageWorker.switchPageByID('MusicManagerPage');
            }
        }
    }

    // Connections {
    //     target: playlistPage
    //     onVisibleChanged: {
    //         if (playlistPage.visible){
    //             if (MediaPlayer.playlist){
    //                 currentPlaylistName = MediaPlayer.playlist.name;
    //                 activeCurrentItem()
    //             }
    //         }
    //     }
    // }

    Component.onCompleted: {
        init()
    }
}