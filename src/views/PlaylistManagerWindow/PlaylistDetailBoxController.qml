import QtQuick 2.3

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
            playMusicByUrl(url);
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