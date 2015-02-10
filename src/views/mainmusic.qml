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

    objectName: "mainMusic"

    BorderImage {
        id: bgIamge
        anchors.fill: mainMusic
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }
    Column{

        Row{

            LeftSideBar {
                id: leftSideBar
                width: 60
                height: mainMusic.height - playBottomBar.height
                iconWidth: leftSideBar.width
                iconHeight: leftSideBar.width
                color: "transparent"
            }

            Column{

                TitleBar {
                    id: titleBar
                    width: mainMusic.width - leftSideBar.width
                    height: 25
                    iconWidth: titleBar.height
                    iconHeight: titleBar.height
                    color: "transparent"
                }

                DStackView {
                    id: stackViews
                    width: mainMusic.width - leftSideBar.width
                    height: mainMusic.height - titleBar.height - playBottomBar.height
                    current: 0
                    WebMusic360Page {
                        id: webMusic360Page
                        width: stackViews.width
                        height: stackViews.height

                        Behavior on opacity {
                            NumberAnimation { duration: 500 }
                        }
                    }

                    MusicManagerPage {
                        id: musicManagerPage
                        width: stackViews.width
                        height: stackViews.height

                        Behavior on opacity {
                            NumberAnimation { duration: 500 }
                        }
                    }

                    PlayListPage {
                        id: palyListPage
                        width: stackViews.width
                        height: stackViews.height

                        Behavior on opacity {
                            NumberAnimation { duration: 500 }
                        }
                    }

                    DownloadPage {
                        id: downPage
                        width: stackViews.width
                        height: stackViews.height

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
            source: "http://sv55.yunpan.cn/Download.outputAudio/1900239694/08c8f061209de7a3840c5e7739cee9e764263c74/55_55.fb6828bec154a5aa381c4d29aa4f6fb6/1.0/openapi/14235691366170/10003/68bbe769b0497c4aab5dea830c60ddbe/小苹果_筷子兄弟.mp3"
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
        // <<M2 
        
        Component.onCompleted: {
            // player.play();
            // if (player.mediaObject)
            //     player.mediaObject.notifyInterval = 50;

            playBottomBar.playButton.played.connect(Utils.playToggle)

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
