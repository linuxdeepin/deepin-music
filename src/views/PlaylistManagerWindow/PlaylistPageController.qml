import QtQuick 2.4

Item {

    property var playlistNavgationBar
    property var playlistDetailBox

    function init() {
        if (ConfigWorker.lastPlaylistName == "favorite"){
            playlistNavgationBar.starList.state = 'Active'
        }else if(ConfigWorker.lastPlaylistName == "temporary"){
            playlistNavgationBar.termporyList.state = 'Active'
        }else{

        }
    }

    Binding { 
        target: playlistNavgationBar.customPlaylistView
        property: 'model'
        value: PlaylistWorker.playlistNames
    }

    Connections {
        target: playlistNavgationBar

        onAddPlaylistName:{
            PlaylistWorker.createPlaylistByName(name);
        }

        onPlaylistNameChanged: {
            var nameId;
            if (name == "我的收藏"){
                nameId = "favorite";
            }else if (name == "试听歌单"){
                nameId = "temporary";
            }else{
                nameId = name;
            }
            MediaPlayer.setPlaylistByName(nameId);
        }
    }

    Component.onCompleted: {
        init()
    }
}