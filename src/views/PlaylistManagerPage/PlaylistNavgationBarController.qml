import QtQuick 2.3


Item {

    property var playlistNavgationBar
    property var addButton
    property var starDelegate
    property var temporaryDelegate
    property var playlistInputText
    property var customPlaylistView

    function init() {
        MenuWorker.playFTAllSongs.connect(checkFTPlaylistName)
        MenuWorker.playNavigationAllSongs.connect(checkNavPlaylistName)
    }

    function checkNavPlaylistName(name, index){
        customPlaylistView.currentIndex = index;
        // customPlaylistView.itemClicked(name);
        starDelegate.state = '!Checked';
        temporaryDelegate.state = '!Checked';
        customPlaylistView.currentItem.state = "Active";
        MediaPlayer.setPlaylistByName(name);
        if (MediaPlayer.playlist.count == 0){
            customPlaylistView.currentItem.state = "!Checked";
        }
    }

    function checkFTPlaylistName(name){
        if (name == 'favorite'){
            starDelegate.leftClicked();
        }else if (name == 'temporary'){
            temporaryDelegate.leftClicked();
        }
        MediaPlayer.setPlaylistByName(name);
    }

    function checkedByName(name) {
         for (var i = 0 ; i< PlaylistWorker.playlistNames.length; i++){
            if (PlaylistWorker.playlistNames[i].name == name){
                customPlaylistView.currentIndex = i;
                customPlaylistView.currentItem.state = "Checked";
                customPlaylistView.itemClicked(name);
                return
            }
        }
    }

    Binding {
        target: customPlaylistView
        property: 'model'
        value: PlaylistWorker.playlistNames
    }

    Connections {
        target: addButton
        onClicked: {
            playlistInputTextBox.visible = true;
            playlistInputText.focus = true
            playlistInputText.text = I18nWorker.newPlaylist + (customPlaylistView.count + 1)
            customPlaylistView.anchors.top =  playlistInputTextBox.bottom;
            customPlaylistView.anchors.topMargin =  14;
        }
    }

    Connections {
        target: playlistInputText
        onAccepted:{
            var text = playlistInputText.text;
            if (text){
                playlistInputTextBox.visible = false;
                playlistInputText.focus = false;
                customPlaylistView.anchors.top =  playlistTitle.bottom;
                customPlaylistView.anchors.topMargin =  0;
                playlistNavgationBar.addPlaylistName(playlistInputText.text);
                checkedByName(playlistInputText.text);
            }
        }
    }

    Connections {
        target: starDelegate
        onLeftClicked: {
            if (temporaryDelegate.state == 'Active'){

            }else {
                temporaryDelegate.state = '!Checked'
            }
            
            customPlaylistView.currentIndex = -1
            playlistNavgationBar.playlistNameChanged(starDelegate.name);
        }

        onRightClicked:{
            MenuWorker.ftPlaylistNavigationMenuShow('favorite');
        }
    }

    Connections {
        target: temporaryDelegate
        onLeftClicked: {
            if (starDelegate.state == 'Active'){

            }else {
                starDelegate.state = '!Checked'
            }
            
            customPlaylistView.currentIndex = -1
            playlistNavgationBar.playlistNameChanged  (temporaryDelegate.name);
        }
        onRightClicked:{
            MenuWorker.ftPlaylistNavigationMenuShow('temporary');
        }
    }

    Connections {
        target: customPlaylistView
        onItemClicked: {
            if (starDelegate.state == 'Active'){

            }else {
                starDelegate.state = '!Checked'
            }
            if (temporaryDelegate.state == 'Active'){

            }else {
                temporaryDelegate.state = '!Checked'
            }
            if (name != playlistNavgationBar._playlistName){
                playlistNavgationBar.playlistNameChanged(name);
                playlistNavgationBar._playlistName = name;
            }
        }

        onMenuShowed:{
            MenuWorker.playlistNavigationMenuShow(name, index);
        }
    }

    Component.onCompleted: {
        init();
    }
}