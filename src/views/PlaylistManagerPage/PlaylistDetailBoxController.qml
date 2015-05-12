import QtQuick 2.3

Item {
    property var playlistDetailBox
    property var playlistView
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

    Binding {
        target: titleText
        property: 'text'
        value: {
            return I18nWorker.song + '   (' + playlistView.count +')'
        }
    }

    Connections {
        target: playlistView

        onPlayMusicByUrl: {
            MediaPlayer.setPlaylistByName(playlistDetailBox.currentPlaylistName);
            playMusicByUrl(songUrl);
        }

        onPlaylistMenuShow: {
            MenuWorker.playlistSongMenuShow(playlistDetailBox.currentPlaylistName, songUrl);
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