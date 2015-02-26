import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.3
import QtWebEngine 1.0
import QtMultimedia 5.0
import DMusic 1.0
// import "Utils.js" as Utils

Rectangle {

    id: mainMusic

    color: "lightgray"
    focus: true

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

                WebEngineViewPage {
                    id: webEngineViewPage

                    objectName: 'webEngineViewPage'

                    width: mainMusic.width - leftSideBar.width
                    height: mainMusic.height - titleBar.height - playBottomBar.height

                    url: "http://10.0.0.153:8093/"
                    Behavior on opacity {
                        NumberAnimation { duration: 500 }
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
    }

    MainController{
        id: mainController
        bgIamge: bgIamge
        titleBar: titleBar
        leftSideBar: leftSideBar
        webEngineViewPage: webEngineViewPage
        playBottomBar: playBottomBar
        player: player
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_F1) {
            mainController.resetSkin();
            // event.accepted = true;
        }else if (event.key == Qt.Key_F2) {
            mainController.setSkinByImage();
            // event.accepted = true;
        }
    }
}
