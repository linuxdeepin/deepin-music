import QtQuick 2.0

Item {
    property var bgIamge
	property var titleBar
	property var leftSideBar
	property var webEngineViewPage
    property var playBottomBar
	property var player

    function initConnect(){

        webEngineViewPage.playMusicByID.connect(Web360ApiWorker.getMusicURLByID)
        Web360ApiWorker.playUrl.connect(playMusic)

        playBottomBar.played.connect(playToggle)
        player.onPlaying.connect(onPlaying)
        player.onPaused.connect(onPaused)
        player.onStopped.connect(onStopped)
        player.onError.connect(onError)
    }

    function playMusic(url){
        player.stop()
        player.source = url;
        console.log(player.metaData.hasOwnProperty('albumArtist'), '---------', player.metaData.albumArtist)
        playToggle(true)
    }

    function playToggle(playing){
        if (playing){
            player.play()
            
            if (player.mediaObject)
               player.mediaObject.notifyInterval = 50;
               console.log(player.metaData.hasOwnProperty('albumArtist'), '++++++++++', player.metaData.albumArtist)
        }else{
            player.pause()
        }
    }

    function onPlaying(){
        playBottomBar.playing = true;
    }

    function onPaused(){
        playBottomBar.playing = false;
    }

    function onStopped(){
        playBottomBar.playing = false;
    }

    function onError(error, errorString){
        playBottomBar.playing = false;
        print(error, errorString);
    }

    function resetSkin() {
        playBottomBar.color = "#282F3F"
        bgIamge.source = ''
    }


    function setSkinByImage(url) {
        if (url === undefined){
            url = "../skin/images/bg2.jpg"
        }
        playBottomBar.color = "transparent"
        bgIamge.source = url
    }

	Connections {
        target: titleBar
        onShowMinimized: MainWindow.showMinimized()
        onMenuShowed: MenuWorker.showSettingMenu()
    }

    Connections {
        target: player
        onMediaObjectChanged: {
            if (player.mediaObject)
                player.mediaObject.notifyInterval = 50;
        }
    }

    Connections {
        target: leftSideBar
        onSwicthViewByID: {
            if (viewID == 'WebMusic360Page'){
                webEngineViewPage.url = "http://10.0.0.153:8093/";
            }
            else if (viewID == 'MusicManagerPage'){
                webEngineViewPage.url = MusicManageWorker.artistUrl
            }
            else if (viewID == 'PlayListPage'){
                webEngineViewPage.url = "https://www.baidu.com";
            }
            else if (viewID == 'DownloadPage'){
                webEngineViewPage.url = "http://www.oschina.net/";
            }else{
                console.log('--------------No Page--------------');
            }
        }
    }

    Component.onCompleted: {
        initConnect();
    }
}
