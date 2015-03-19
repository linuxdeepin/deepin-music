import QtQuick 2.4

Item {
    property var rootWindow
    property var simpleWindow
    property var constants
    property var bgImage
	property var titleBar
    property var switchButton
    property var stackViews
    property var playlistPage
    property var lrcPage
	property var playBottomBar

    function resetSkin() {
        playBottomBar.color = "transparent";
        bgImage.source = constants.defaultBackgroundImage;
    }

    function setSkinByImage(url) {
        if(url){
            bgImage.source = url;
        }else{
            resetSkin();
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
    //     target: playBottomBar.volumeButton
    //     property: 'switchflag'
    //     value: !MediaPlayer.muted
    // }

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
            switchButton.switchflag = !switchButton.switchflag;
            var index =  1 - stackViews.currentIndex
            stackViews.setCurrentIndex(index);
        }
    }

    Connections {
        target: titleBar

        onMainWindowShowed: {
            rootWindow.mainController.showMainWindow();
        }

        onMenuShowed: {
            MenuWorker.showSettingMenu();
        }
        

        onShowMinimized: {
            MainWindow.showMinimized();
        }

        onClosed: {
            Qt.quit();
        }
    }

    Connections {
        target: playBottomBar

        onPreMusic: MediaPlayer.previous()

        onPlayed: MediaPlayer.playToggle(isPlaying)

        onNextMusic: MediaPlayer.next()

        onVolumeChanged: {
            MediaPlayer.volume = parseInt(value * 100)
        }
        onMuted: {
            MediaPlayer.muted = muted;
            rootWindow.mainWindow.playBottomBar.volumeButton.switchflag = !muted;
        }

        onPlaybackModeChanged:{
           MediaPlayer.setPlaybackMode(playbackMode);
        }
    }

    Connections {
        target: playBottomBar.slider
        onSliderRateChanged:{
            if (MediaPlayer.seekable){
                MediaPlayer.muted = true;
                rootWindow.positionTimer.restart();
                MediaPlayer.setPosition(MediaPlayer.duration * rate)
            }
        }
    }


   Component.onCompleted: {

        var mainPlayBottomBar = rootWindow.mainWindow.playBottomBar;
        playBottomBar.slider.value = mainPlayBottomBar.slider.value;
        playBottomBar.volumeButton.switchflag = !MediaPlayer.muted;
        playBottomBar.volumeSlider.value = mainPlayBottomBar.volumeSlider.value;
        playBottomBar.cycleButton.playbackMode = mainPlayBottomBar.cycleButton.playbackMode;
        playBottomBar.playing = mainPlayBottomBar.playing
    }
}
