import QtQuick 2.0

Item {
    property var bgImage
	property var titleBar
	property var playBottomBar

	
    function initConnect(){
        // playBottomBar.played.connect(playToggle)
        MediaPlayer.positionChanged.connect(updateSlider)
        MediaPlayer.stateChanged.connect(updatePlayBar)
    }

    function playMusic(url){
        player.stop()
        player.source = url;
        playToggle(true)
    }

    function playToggle(playing){
        if (playing){
            MediaPlayer.play()
            if (MediaPlayer.mediaObject){
               MediaPlayer.setNotifyInterval(50);
            }
        }else{
            MediaPlayer.pause()
        }
    }

    function updateSlider(position) {
        var rate = position / MediaPlayer.duration;
        playBottomBar.slider.updateSlider(rate);
    }

    function updatePlayBar(state) {
        if (state == 0){
            onStopped();
        }else if (state == 1){
            onPlaying();
        }else if (state == 2){
            onPaused();
        }
    }

    function onPlaying(){
        playBottomBar.playing = true;
        console.log('Playing')
    }

    function onPaused(){
        playBottomBar.playing = false;
        console.log('Paused')
    }

    function onStopped(){
        playBottomBar.playing = false;
        console.log('Stopped')
    }

	Connections {
        target: titleBar
        onShowMinimized: SimpleWindow.showMinimized()
        onMenuShowed: MenuWorker.showSettingMenu()
        onMainWindowShowed: WindowManageWorker.mainWindowShowed()
    }

    Connections {
        target: playBottomBar.slider
        onSliderRateChanged:{
            if (MediaPlayer.seekable){
                MediaPlayer.setPosition(MediaPlayer.duration * rate)
            }
        }
    }

    Component.onCompleted: {
        initConnect();
    }
}
