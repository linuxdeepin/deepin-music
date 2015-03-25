import QtQuick 2.4

Item {

	property var playlistNavgationBar
    property var playlistDetailBox


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
}