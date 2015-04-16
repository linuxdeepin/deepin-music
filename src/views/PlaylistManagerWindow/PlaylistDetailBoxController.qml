import QtQuick 2.4

Item {
    property var playlistDetailBox
    property var playlistView
    property var titleText
    property var linkTipText

    function init() {
    }

    function playMusicByUrl(url) {
        if (url.indexOf('http') != -1){
            Web360ApiWorker.playMediaByUrl(url);
        }else{
            MediaPlayer.playLocalMedia(url);
        }
    }

    // function activeCurrentItem(currentIndex){
    //     if (MediaPlayer.playlist) {
    //         if (MediaPlayer.playlist.name == currentPlaylistName){
    //             playlistView.currentIndex = MediaPlayer.playlist.currentIndex;
    //             if (playlistView.currentItem){
    //                 if (MediaPlayer.playing){
    //                     playlistView.currentItem.state = "Active";
    //                 }else{
    //                     playlistView.currentItem.state = "!Checked";
    //                 }
    //             }
    //         }

    //         if (MediaPlayer.playing){
    //             if (MediaPlayer.playlist.name == 'favorite'){
    //                 playlistNavgationBar.starDelegate.state = 'Active'
    //                 if (currentPlaylistName == 'temporary'){
    //                     playlistNavgationBar.temporaryDelegate.state = 'Checked'
    //                 }else{
    //                     playlistNavgationBar.temporaryDelegate.state = '!Checked'
    //                 }

    //             }else if(MediaPlayer.playlist.name == 'temporary'){
    //                 playlistNavgationBar.temporaryDelegate.state = 'Active'
    //                 if (currentPlaylistName == 'favorite'){
    //                     playlistNavgationBar.starDelegate.state = 'Checked'
    //                 }else{
    //                     playlistNavgationBar.starDelegate.state = '!Checked'
    //                 }
    //             }else{

    //             }
    //         }else{
    //            if (currentPlaylistName == 'favorite'){
    //                 playlistNavgationBar.starDelegate.state = 'Checked'
    //                 playlistNavgationBar.temporaryDelegate.state = '!Checked'

    //             }else if(currentPlaylistName == 'temporary'){
    //                 playlistNavgationBar.starDelegate.state = '!Checked'
    //                 playlistNavgationBar.temporaryDelegate.state = 'Checked'
    //             }else{

    //             }
    //         }
    //     }
    // }

    Binding {
        target: titleText
        property: 'text'
        value: {
            if (playlistView.model) {
                var model = playlistView.model;
                return I18nWorker.song + '   (' + model.length +')'
            }
        }
    }

    Connections {
        target: playlistView

        onPlayMusicByUrl: {
            MediaPlayer.setPlaylistByName(playlistDetailBox.currentPlaylistName);
            playMusicByUrl(url);

            // if (MediaPlayer.playlist.name == 'favorite'){
            //     playlistNavgationBar.starDelegate.state = 'Active'
            //     playlistNavgationBar.temporaryDelegate.state = '!Checked'

            // }else if(MediaPlayer.playlist.name == 'temporary'){
            //     playlistNavgationBar.starDelegate.state = '!Checked'
            //     playlistNavgationBar.temporaryDelegate.state = 'Active'
            // }else{

            // }
        }

        onModelChanged: {
            playlistView.positionViewAtEnd()
            playlistView.currentIndex = -1;
            activeCurrentItem();
        }
    }

    
    Connections {
        target: linkTipText
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