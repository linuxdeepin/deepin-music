import QtQuick 2.3

Item {
    property var playlistDetailBox
    property var playlistView
    property var noMusicTip
    property var titleText
    property var linkTipText

    function init() {
        MenuWorker.playMusicByUrl.connect(playlistView.playMusicByUrl);
    }

    function playMusicByUrl(songUrl) {
        if (songUrl.indexOf('http') != -1){
            Web360ApiWorker.playMediaByUrl(songUrl);
        }else{
            MediaPlayer.playLocalMedia(songUrl);
        }
    }

    function getModelByPlaylistName(name){
        try {
            if (name){
                var model = eval('Playlist_' + Qt.md5(name));
                if (model){
                    return model
                }else{
                    return EmptyModel
                }
            }else{
                return EmptyModel
            }
        }catch(e){
            return EmptyModel
        }
    }

    Binding {
        target: noMusicTip
        property: 'visible'
        value: {
            var model = getModelByPlaylistName(playlistDetailBox.currentPlaylistName);
            if (model.count == 0){
                return true;
            }else{
                return false;
            }
        }
    }

    Binding {
        target: playlistView
        property: 'currentIndex'
        value: {
            var playlist = MediaPlayer.playlist;
            if (playlist){
                if (playlist && playlist.name == playlistDetailBox.currentPlaylistName){
                    return playlist.currentIndex;
                }else{
                    return -1;
                }
            }
            else{
                return -1;
            }
        }
    }

    Connections {
        target: playlistView

        onPlayMusicByUrl: {
            MediaPlayer.setPlaylistByName(playlistDetailBox.currentPlaylistName);
            playMusicByUrl(songUrl);
        }

        onLocalMenuShowed: {
            MenuWorker.playlistLocalSongMenuShow(playlistDetailBox.currentPlaylistName, songUrl);
        }

        onOnlineMenuShowed: {
            MenuWorker.playlistOnlineSongMenuShow(playlistDetailBox.currentPlaylistName, songUrl, songId);
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

    Component.onCompleted: {
        init()
    }
}