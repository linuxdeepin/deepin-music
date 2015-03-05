import QtQuick 2.4

Item {
    property var mainWindow
    property var simpleWindow
    property var miniWindow

    function initConnect(){
        MediaPlayer.positionChanged.connect(updateSlider);
        MediaPlayer.positionChanged.connect(updateMusicTime);
        MediaPlayer.stateChanged.connect(updatePlayButton);
        MediaPlayer.mediaStatusChanged.connect(updateMusic);
    }

    function updateSlider(position) {
        var rate = position / MediaPlayer.duration;

        mainWindow.playBottomBar.slider.updateSlider(rate);
        simpleWindow.playBottomBar.slider.updateSlider(rate);
        miniWindow.slider.updateSlider(rate);
    }

    function updateMusicTime(position){
        mainWindow.playBottomBar.updatePlayTime(MediaPlayer.positionString + '/' + MediaPlayer.durationString);
        simpleWindow.playBottomBar.updatePlayTime(MediaPlayer.positionString + '/' + MediaPlayer.durationString);
    }

    function audioAvailableChange(available)
    {
        print('audioAvailableChanged---------')
    }

    function updateMusicInfo()
    {
        mainWindow.playBottomBar.updateMusicName(MediaPlayer.metaData('Title'));
        mainWindow.playBottomBar.updateArtistName(MediaPlayer.metaData('ContributingArtist'));
        
        simpleWindow.playBottomBar.updateMusicName(MediaPlayer.metaData('Title'));
        simpleWindow.playBottomBar.updateArtistName(MediaPlayer.metaData('ContributingArtist'));
    }


    function updateMusic(status){
        print('mediaStatusChanged', status)
        if (status == 3 || status==6) {
            print('The current media has been loaded')
            updateMusicInfo()
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

    function resetSkin() {
        playBottomBar.color = "#282F3F"
        bgImage.source = ''
    }


    function setSkinByImage(url) {
        if (url === undefined){
            url = "../skin/images/bg2.jpg"
        }
        playBottomBar.color = "transparent"
        bgImage.source = url
    }


    Connections {
        target: mainWindow.playBottomBar.slider
        onSliderRateChanged:{
            if (MediaPlayer.seekable){
                MediaPlayer.setPosition(MediaPlayer.duration * rate)
            }
        }
    }

    Connections {
        target: simpleWindow.playBottomBar.slider
        onSliderRateChanged:{
            if (MediaPlayer.seekable){
                MediaPlayer.setPosition(MediaPlayer.duration * rate)
            }
        }
    }

    Connections {
        target: miniWindow.slider
        onSliderRateChanged:{
            if (MediaPlayer.seekable){
                MediaPlayer.setPosition(MediaPlayer.duration * rate)
            }
        }
    }

    Connections {
        target: mainWindow.playBottomBar

        onPreMusic: PlaylistWorker.previous()

        onPlayed: MediaPlayer.playToggle(isPlaying)

        onNextMusic: PlaylistWorker.next()
    }

    Connections {
        target: simpleWindow.playBottomBar

        onPreMusic: PlaylistWorker.previous()

        onPlayed: MediaPlayer.playToggle(isPlaying)

        onNextMusic: PlaylistWorker.next()
    }

    Connections {
        target: miniWindow

        onPreMusic: PlaylistWorker.previous()

        onPlayed: MediaPlayer.playToggle(isPlaying)

        onNextMusic: PlaylistWorker.next()
    }

    Component.onCompleted: {
        initConnect();
    }
}
