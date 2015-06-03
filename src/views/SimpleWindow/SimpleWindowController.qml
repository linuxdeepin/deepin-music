import QtQuick 2.3

Item {
    property var simpleWindow
    property var bgImage
	property var titleBar
    property var switchButton
    property var stackViews
    property var playlistPage
    property var lrcPage
	property var playBottomBar


    function initConnect(){
        MediaPlayer.positionChanged.connect(updateSlider);
        MediaPlayer.positionChanged.connect(updateMusicTime);
        MediaPlayer.stateChanged.connect(updatePlayButton);
        MediaPlayer.volumeChanged.connect(updateVolumeSlider);
        MediaPlayer.playbackModeChanged.connect(updateCycleButton);
        MediaPlayer.bufferStatusChanged.connect(updateBufferSlider);
        MediaPlayer.currentIndexChanged.connect(updatePlaylistIndex);
        MediaPlayer.coverChanged.connect(updateBackgroundCover);
    }


    function disconnect(){
        MediaPlayer.positionChanged.disconnect(updateSlider);
        MediaPlayer.positionChanged.disconnect(updateMusicTime);
        MediaPlayer.stateChanged.disconnect(updatePlayButton);
        MediaPlayer.volumeChanged.disconnect(updateVolumeSlider);
        MediaPlayer.playbackModeChanged.disconnect(updateCycleButton);
        MediaPlayer.bufferStatusChanged.disconnect(updateBufferSlider);
        MediaPlayer.currentIndexChanged.disconnect(updatePlaylistIndex);
        MediaPlayer.coverChanged.disconnect(updateBackgroundCover);
    }

    function resetSkin() {
        playBottomBar.color = "transparent";
        bgImage.source = Qt.constants.defaultBackgroundImage;
    }

    function setSkinByImage(url) {
        if(url){
            bgImage.source = url;
        }else{
            resetSkin();
        }
    }

    function updateSlider(position) {
        var rate = position / MediaPlayer.duration;
        if (playBottomBar){
            playBottomBar.slider.updateSlider(rate);
        }
    }

    function updateBufferSlider(position){
        var rate = position / 100;
        if (playBottomBar){
            playBottomBar.slider.updateBufferSlider(rate);
            if (position == 100){
                playBottomBar.slider.updateBufferSlider(0);
            }
        }
    }

    function updateMusicTime(position){
        if (playBottomBar) {
            playBottomBar.updatePlayTime(MediaPlayer.positionString + '/' + MediaPlayer.durationString);
        }
    }

    function updateBackgroundCover(cover){
        if(ConfigWorker.isCoverBackground){
            simpleWindowController.setSkinByImage(cover);
        }
    }

    function updatePlayButton(state) {
        if (state == 0){
            onStopped();
        }else if (state == 1){
            onPlaying();
        }else if (state == 2){
            onPaused();
        }
    }

    function onPlaying(){
        if (playBottomBar){
            playBottomBar.playing = true;
        }
    }

    function onPaused(){
        if(playBottomBar) {
            playBottomBar.playing = false;
        }
    }

    function onStopped(){
        if (playBottomBar) {
            playBottomBar.playing = false;
        }
    }

    function updateVolumeSlider(value){
        if (playBottomBar) {
            playBottomBar.volumeSlider.value = value / 100;
        }
    }

    function updateCycleButton(value){
        if (playBottomBar) {
            playBottomBar.cycleButton.playbackMode = value;
        }
    }

    function updatePlaylistIndex(index){
        if (playlistPage) {
            playlistPage.playlistView.currentIndex = index;
        }
    }

    function playMusicByUrl(url) {
        if (url.indexOf('http') != -1){
            Web360ApiWorker.playMediaByUrl(url);
        }else{
            MediaPlayer.playLocalMedia(url);
        }
    }

    Binding { 
        target: playBottomBar
        property: 'title'
        value: MediaPlayer.title
    }

    Binding { 
        target: playBottomBar
        property: 'artist'
        value: MediaPlayer.artist
    }
    Binding { 
        target: playBottomBar
        property: 'cover'
        value: MediaPlayer.cover
    }


    // Binding {
    //     target: playlistPage.playlistView
    //     property: 'model'
    //     value: {
    //         var playlist = MediaPlayer.playlist;
    //         if (playlist){
    //             return playlist.medias;
    //         }else{
    //             return null;
    //         }
    //     }
    // }

    Binding {
        target: playlistPage.playlistView
        property: 'currentIndex'
        value:  {
            var playlist = MediaPlayer.playlist;
            if (playlist){
                return playlist.currentIndex;
            }else{
                return -1;
            }
        }
    }

    Connections {
        target: bgImage
        onProgressChanged:{
            if (progress == 1){
                playBottomBar.color = 'transparent'
            }
        }
    }

    Connections {
        target: switchButton
        onClicked: {
            var index =  1 - stackViews.currentIndex
            stackViews.setCurrentIndex(index);
        }
    }

    Connections {
        target: titleBar

        onMainWindowShowed: {
            Qt.mainController.showMainWindow();
        }

        onMenuShowed: {
            MenuWorker.settingMenuShow('SimpleWindow');
        }
        

        onShowMinimized: {
            MainWindow.showMinimized();
        }

        onClosed: {
            Qt.quit();
        }
    }

    Connections {
        target: playlistPage.playlistView
        onPlayMusicByUrl: playMusicByUrl(url)
        onModelChanged: playlistPage.playlistView.positionViewAtEnd()
    }

    Connections {
        target: playBottomBar

        onPreMusic: MediaPlayer.previous()

        onPlayed: MediaPlayer.playToggle(isPlaying)

        onNextMusic: MediaPlayer.next()

        onVolumeChanged: {
            MediaPlayer.volume = Math.round(value * 100)
        }
        onMuted: {
            MediaPlayer.muted = muted;
            Qt.mainWindow.playBottomBar.volumeButton.switchflag = !muted;
        }

        onPlaybackModeChanged:{
           MediaPlayer.playbackMode = playbackMode;
        }
    }

    Connections {
        target: playBottomBar.slider
        onSliderRateChanged:{
            if (MediaPlayer.seekable){
                MediaPlayer.muted = true;
                Qt.positionTimer.restart();
                MediaPlayer.position = MediaPlayer.duration * rate
            }
        }
    }

    Component.onCompleted: {
        playBottomBar.volumeButton.switchflag = !MediaPlayer.muted;
        playBottomBar.volumeSlider.value = MediaPlayer.volume / 100;
        playBottomBar.cycleButton.playbackMode = MediaPlayer.playbackMode;
        playBottomBar.playing = MediaPlayer.playing;
        playBottomBar.slider.value = MediaPlayer.position / MediaPlayer.duration;
        initConnect();
    }

    Component.onDestruction: {
        disconnect();
    }
}
