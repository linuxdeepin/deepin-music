import QtQuick 2.4

Item {
    property var mainWindow
    property var simpleWindow
    property var miniWindow
    property var positionTimer

    function initConnect(){
        MediaPlayer.positionChanged.connect(updateSlider);
        MediaPlayer.positionChanged.connect(updateMusicTime);
        MediaPlayer.stateChanged.connect(updatePlayButton);
        MediaPlayer.volumeChanged.connect(updateVolumeSlider);
        MediaPlayer.playbackModeChanged.connect(updateCycleButton);
        MediaPlayer.musicInfoChanged.connect(updateMusicInfo);
        MediaPlayer.bufferStatusChanged.connect(updateBufferSlider);

        MediaPlayer.setPlaylistByName(ConfigWorker.lastPlaylistName);
        MediaPlayer.playlist.setCurrentIndex(ConfigWorker.lastPlayedIndex);
        MediaPlayer.volumeChanged(ConfigWorker.volume);
        MediaPlayer.setPlaybackMode(ConfigWorker.playbackMode);

    }

    function updateSlider(position) {
        var rate = position / MediaPlayer.duration;

        mainWindow.playBottomBar.slider.updateSlider(rate);
        simpleWindow.playBottomBar.slider.updateSlider(rate);
        miniWindow.slider.updateSlider(rate);
    }

    function updateBufferSlider(position){
        var rate = position / 100;

        mainWindow.playBottomBar.slider.updateBufferSlider(rate);
        simpleWindow.playBottomBar.slider.updateBufferSlider(rate);
        miniWindow.slider.updateBufferSlider(rate);

        if (position == 100){
            mainWindow.playBottomBar.slider.updateBufferSlider(0);
            simpleWindow.playBottomBar.slider.updateBufferSlider(0);
            miniWindow.slider.updateBufferSlider(0);
        }
    }

    function updateMusicTime(position){
        mainWindow.playBottomBar.updatePlayTime(MediaPlayer.positionString + '/' + MediaPlayer.durationString);
        simpleWindow.playBottomBar.updatePlayTime(MediaPlayer.positionString + '/' + MediaPlayer.durationString);
    }

    function updateMusicInfo(title, artist, cover)
    {
        mainWindow.playBottomBar.updateCoverImage(cover);
        mainWindow.playBottomBar.updateMusicName(title);
        mainWindow.playBottomBar.updateArtistName(artist);

        simpleWindow.playBottomBar.updateCoverImage(cover);
        simpleWindow.playBottomBar.updateMusicName(title);
        simpleWindow.playBottomBar.updateArtistName(artist);

        if(ConfigWorker.isCoverBackground){
            mainWindow.mainWindowController.setSkinByImage(cover);
            simpleWindow.simpleWindowController.setSkinByImage(cover);
            miniWindow.miniWindowController.setSkinByImage(cover);
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
        mainWindow.playBottomBar.playing = true;
        simpleWindow.playBottomBar.playing = true;
        miniWindow.playing = true;
        console.log('Playing');
    }

    function onPaused(){
        mainWindow.playBottomBar.playing = false;
        simpleWindow.playBottomBar.playing = false;
        miniWindow.playing = false;
        console.log('Paused')
    }

    function onStopped(){
        mainWindow.playBottomBar.playing = false;
        simpleWindow.playBottomBar.playing = false;
        miniWindow.playing = false;
        console.log('Stopped')
    }

    function updateVolumeSlider(value){
        mainWindow.playBottomBar.volumeSlider.value = value / 100;
        simpleWindow.playBottomBar.volumeSlider.value = value / 100;
    }

    function updateCycleButton(value){
        mainWindow.playBottomBar.cycleButton.playbackMode = value;
        simpleWindow.playBottomBar.cycleButton.playbackMode = value;
    }

    Connections {
        target: mainWindow.playBottomBar.slider
        onSliderRateChanged:{
            if (MediaPlayer.seekable){
                MediaPlayer.setMuted(true);
                positionTimer.restart();
                MediaPlayer.setPosition(MediaPlayer.duration * rate)
            }
        }
    }

    Connections {
        target: simpleWindow.playBottomBar.slider
        onSliderRateChanged:{
            if (MediaPlayer.seekable){
                MediaPlayer.setMuted(true);
                positionTimer.restart();
                MediaPlayer.setPosition(MediaPlayer.duration * rate)
            }
        }
    }

    Connections {
        target: miniWindow.slider
        onSliderRateChanged:{
            if (MediaPlayer.seekable){
                MediaPlayer.setMuted(true);
                positionTimer.restart();
                MediaPlayer.setPosition(MediaPlayer.duration * rate)
            }
        }
    }

    Connections {
        target: mainWindow.playBottomBar

        onPreMusic: MediaPlayer.previous()

        onPlayed: MediaPlayer.playToggle(isPlaying)

        onNextMusic: MediaPlayer.next()

        onVolumeChanged: {
            MediaPlayer.setVolume(parseInt(value * 100));
        }

        onMuted: {
            MediaPlayer.setMuted(muted);
            simpleWindow.playBottomBar.volumeButton.switchflag = !muted;
        }

        onPlaybackModeChanged:{
            MediaPlayer.setPlaybackMode(playbackMode);
        }
    }

    Connections {
        target: simpleWindow.playBottomBar

        onPreMusic: MediaPlayer.previous()

        onPlayed: MediaPlayer.playToggle(isPlaying)

        onNextMusic: MediaPlayer.next()

        onVolumeChanged: {
            MediaPlayer.setVolume(parseInt(value * 100))
        }
        onMuted: {
            MediaPlayer.setMuted(muted);
            mainWindow.playBottomBar.volumeButton.switchflag = !muted;
        }

        onPlaybackModeChanged:{
           MediaPlayer.setPlaybackMode(playbackMode);
        }
    }

    Connections {
        target: simpleWindow.playlistPage.playlistView
        onChangeIndex: {
            MediaPlayer.setCurrentMedia(index);
            MediaPlayer.playToggle(true);
        }
    }

    Connections {
        target: miniWindow

        onPreMusic: MediaPlayer.previous()

        onPlayed: MediaPlayer.playToggle(isPlaying)

        onNextMusic: MediaPlayer.next()
    }

    Connections {
        target: positionTimer
        onTriggered:{
            MediaPlayer.setMuted(false);
        } 
    }

    Component.onCompleted: {
        initConnect();
    }
}
