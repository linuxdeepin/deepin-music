import QtQuick 2.0

Item {
    property var simpleWindow
    property var bgImage
	property var titleBar
	property var playBottomBar
    
	
    function initConnect(){
        MediaPlayer.positionChanged.connect(updateSlider)
        MediaPlayer.stateChanged.connect(updatePlayBar)
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
        target: playBottomBar.slider
        onSliderRateChanged:{
            if (MediaPlayer.seekable){
                MediaPlayer.setPosition(MediaPlayer.duration * rate)
            }
        }
    }


    Connections {
        target: playBottomBar

        onPreMusic: PlaylistWorker.previous()

        onPlayed: MediaPlayer.playToggle(isPlaying)

        onNextMusic: PlaylistWorker.next()
    }



    Component.onCompleted: {
        initConnect();
    }
}
