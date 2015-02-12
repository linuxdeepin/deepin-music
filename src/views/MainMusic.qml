import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtWebEngine 1.0
import QtMultimedia 5.0
import DMusic 1.0
import "Utils.js" as Utils

Rectangle {

    id: mainMusic
    
    property var views: ['WebMusic360Page', 'MusicManagerPage', 'PlayListPage', 'DownloadPage']

    color: "lightgray"
    focus: true

    function initConnect(){

        webMusic360Page.playMusicByID.connect(Web360ApiWorker.getMusicURLByID)
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
        Utils.playToggle(true)
    }

    function playToggle(playing){
        if (playing){
            player.play()
            if (player.mediaObject)
               player.mediaObject.notifyInterval = 50;
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

    BorderImage {
        id: bgIamge
        objectName: 'bgImage'

        anchors.fill: mainMusic
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }
    Column{

        Row{

            LeftSideBar {
                id: leftSideBar
                objectName: 'leftSideBar'
                width: 60
                height: mainMusic.height - playBottomBar.height
                iconWidth: leftSideBar.width
                iconHeight: leftSideBar.width
                color: "transparent"
            }

            Column{

                TitleBar {
                    id: titleBar
                    objectName: 'titleBar'
                    width: mainMusic.width - leftSideBar.width
                    height: 25
                    iconWidth: titleBar.height
                    iconHeight: titleBar.height
                    color: "transparent"
                }

                DStackView {
                    id: stackViews
                    objectName: 'stackViews'
                    width: mainMusic.width - leftSideBar.width
                    height: mainMusic.height - titleBar.height - playBottomBar.height
                    current: 0
                    WebMusic360Page {
                        id: webMusic360Page

                        objectName: 'webMusic360Page'
                        anchors.fill: parent

                        Behavior on opacity {
                            NumberAnimation { duration: 500 }
                        }
                    }

                    MusicManagerPage {
                        id: musicManagerPage

                        objectName: 'musicManagerPage'
                        anchors.fill: parent

                        Behavior on opacity {
                            NumberAnimation { duration: 500 }
                        }
                    }

                    PlayListPage {
                        id: palyListPage

                        objectName: 'palyListPage'
                        anchors.fill: parent

                        Behavior on opacity {
                            NumberAnimation { duration: 500 }
                        }
                    }

                    DownloadPage {
                        id: downloadPage

                        objectName: 'downloadPage'
                        anchors.fill: parent

                        Behavior on opacity {
                            NumberAnimation { duration: 500 }
                        }
                    }
                }
            }
        }

        PlayBottomBar {
            id: playBottomBar
            playerDuration: player.duration
            playerPosition: player.position
            width: mainMusic.width
            height: 100

            color: "#282F3F"

            Component.onCompleted: {
            }
        }

        MediaPlayer {
            id: player
            volume: 1
            source: ''
        }

        // VideoOutput {    
        //     source: player
        // }

        Connections {
            target: player
            onMediaObjectChanged: {
                if (player.mediaObject)
                    player.mediaObject.notifyInterval = 50;
            }
        }
    }


    Keys.onPressed: {
        if (event.key == Qt.Key_F1) {
            Utils.resetSkin();
            // event.accepted = true;
        }else if (event.key == Qt.Key_F2) {
            Utils.setSkinByImage();
            // event.accepted = true;
        }
    }

    Connections {
        target: leftSideBar
        onSwicthViewByID: {
            var index =  mainMusic.views.indexOf(viewID);
            stackViews.setCurrentIndex(index);
        }
    }
}
