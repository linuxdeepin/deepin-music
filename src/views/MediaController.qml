import QtQuick 2.3
import "./TemporaryWindow"

Item {
    property var mainWindow
    property var miniWindow
    property var positionTimer

    function initConnect(){
        MediaPlayer.positionChanged.connect(updateSlider);
        MediaPlayer.positionChanged.connect(updateMusicTime);
        MediaPlayer.stateChanged.connect(updatePlayButton);
        MediaPlayer.volumeChanged.connect(updateVolumeSlider);
        MediaPlayer.playbackModeChanged.connect(updateCycleButton);
        MediaPlayer.bufferStatusChanged.connect(updateBufferSlider);
        MediaPlayer.coverChanged.connect(updateBackgroundCover);
    }

    function updateSlider(position) {
        var rate = position / MediaPlayer.duration;
        mainWindow.playBottomBar.slider.updateSlider(rate);
        miniWindow.slider.updateSlider(rate);
    }

    function updateBufferSlider(position){
        var rate = position / 100;
        mainWindow.playBottomBar.slider.updateBufferSlider(rate);
        miniWindow.slider.updateBufferSlider(rate);

        if (position == 100){
            mainWindow.playBottomBar.slider.updateBufferSlider(0);
            miniWindow.slider.updateBufferSlider(0);
        }
    }

    function updateMusicTime(position){
        mainWindow.playBottomBar.updatePlayTime(MediaPlayer.positionString + '/' + MediaPlayer.durationString);
    }

    function updateBackgroundCover(cover){
        if(ConfigWorker.isCoverBackground){
            mainWindow.mainWindowController.setSkinByImage(cover);
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
        miniWindow.playing = true;
    }

    function onPaused(){
        mainWindow.playBottomBar.playing = false;
        miniWindow.playing = false;
    }

    function onStopped(){
        mainWindow.playBottomBar.playing = false;
        miniWindow.playing = false;
    }

    function updateVolumeSlider(value){
        mainWindow.playBottomBar.volumeSlider.value = value / 100;
    }

    function updateCycleButton(value){
        mainWindow.playBottomBar.cycleButton.playbackMode = value;
    }

    function clearCurrentPlaylist() {
        MediaPlayer.playlist.clearMedias()
    }

    Binding { 
        target: mainWindow.playBottomBar
        property: 'title'
        value: MediaPlayer.title
    }

    Binding { 
        target: mainWindow.playBottomBar
        property: 'artist'
        value: MediaPlayer.artist
    }

    Binding { 
        target: mainWindow.playBottomBar
        property: 'cover'
        value: MediaPlayer.cover
    }

    Connections {
        target: mainWindow.playBottomBar.slider
        onSliderRateChanged:{
            if (MediaPlayer.seekable){
                MediaPlayer.muted = true;
                positionTimer.restart();
                MediaPlayer.position = MediaPlayer.duration * rate
            }
        }
    }

    Connections {
        target: miniWindow.slider
        onSliderRateChanged:{
            if (MediaPlayer.seekable){
                MediaPlayer.muted = true;
                positionTimer.restart();
                MediaPlayer.position = MediaPlayer.duration * rate
            }
        }
    }

    Connections {
        target: mainWindow.playBottomBar

        onPreMusic: MediaPlayer.previous()

        onPlayed: MediaPlayer.playToggle(isPlaying)

        onNextMusic: MediaPlayer.next()

        onVolumeChanged: {
            MediaPlayer.volume = Math.round(value * 100);
        }

        onMuted: {
            MediaPlayer.muted = muted;
        }

        onPlaybackModeChanged:{
            MediaPlayer.playbackMode = playbackMode;
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
            MediaPlayer.muted = false;
        } 
    }

    Component.onCompleted: {
        initConnect();
    }
}
