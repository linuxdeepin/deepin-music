import QtQuick 2.4
import "./TermporyWindow"

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
        MediaPlayer.bufferStatusChanged.connect(updateBufferSlider);
        MediaPlayer.currentIndexChanged.connect(updatePlaylistIndex);
        MediaPlayer.coverChanged.connect(updateBackgroundCover);
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

    function updateBackgroundCover(cover){
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
        // console.log('Playing');
    }

    function onPaused(){
        mainWindow.playBottomBar.playing = false;
        simpleWindow.playBottomBar.playing = false;
        miniWindow.playing = false;
        // console.log('Paused')
    }

    function onStopped(){
        mainWindow.playBottomBar.playing = false;
        simpleWindow.playBottomBar.playing = false;
        miniWindow.playing = false;
        // console.log('Stopped')
    }

    function updateVolumeSlider(value){
        mainWindow.playBottomBar.volumeSlider.value = value / 100;
        simpleWindow.playBottomBar.volumeSlider.value = value / 100;
    }

    function updateCycleButton(value){
        mainWindow.playBottomBar.cycleButton.playbackMode = value;
        simpleWindow.playBottomBar.cycleButton.playbackMode = value;
    }

    function updatePlaylistIndex(index){
        simpleWindow.playlistPage.playlistView.currentIndex = index;
        if (mainWindow.termporyLoader.item){
            mainWindow.termporyLoader.item.playlistView.currentIndex = index;
        }
    }

    function changeIndex(index) {
        MediaPlayer.setCurrentMedia(index);
        MediaPlayer.playToggle(true);
    }

    function positionEnd(){
        simpleWindow.playlistPage.playlistView.positionViewAtEnd();
        if (mainWindow.termporyLoader.item){
            mainWindow.termporyLoader.item.playlistView.positionViewAtEnd();
        }
    }

    function destoryTermporyWindow(){
        mainWindow.termporyLoader.source = ''
        mainWindow.focus = true;
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

    Binding { 
        target: simpleWindow.playBottomBar
        property: 'title'
        value: MediaPlayer.title
    }

    Binding { 
        target: simpleWindow.playBottomBar
        property: 'artist'
        value: MediaPlayer.artist
    }
    Binding { 
        target: simpleWindow.playBottomBar
        property: 'cover'
        value: MediaPlayer.cover
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
            changeIndex(index);
        }
    }

    Connections {
        target: simpleWindow.playlistPage.playlistView
        onModelChanged:{
            simpleWindow.playlistPage.playlistView.positionViewAtEnd()
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

    Connections {
        target: mainWindow.playBottomBar.playlistButton
        onClicked:{
            if(mainWindow.termporyLoader.source == ''){
                mainWindow.termporyLoader.source = './TermporyWindow/TermporyWindow.qml'
                mainWindow.termporyLoader.focus = true;
                mainWindow.focus = false;
            }else{
                destoryTermporyWindow();
            }
        } 
    }

    Connections {
        target: mainWindow.termporyLoader
        onLoaded:{
            var item = mainWindow.termporyLoader.item;
            var playlistView =item.playlistView;
            var closeButton = item.closeButton;
            var clearButton = item.clearButton;
            playlistView.changeIndex.connect(changeIndex);
            playlistView.modelChanged.connect(positionEnd);
            closeButton.clicked.connect(destoryTermporyWindow);
            clearButton.clicked.connect(clearCurrentPlaylist);
        } 
    }

    Component.onCompleted: {
        initConnect();
    }
}
