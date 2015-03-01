import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.3
import QtWebEngine 1.0
import QtMultimedia 5.0
import DMusic 1.0


Rectangle {

    id: mainWindow

    property var titleBar: titleBar
    property var mainWindowController: mainWindowController

    color: "lightgray"
    focus: true

    BorderImage {
        id: bgImage
        objectName: 'bgImage'

        anchors.fill: mainWindow
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }
    Column{

        Row{

            LeftSideBar {
                id: leftSideBar
                objectName: 'leftSideBar'
                width: 60
                height: mainWindow.height - playBottomBar.height
                iconWidth: leftSideBar.width
                iconHeight: leftSideBar.width
                color: "transparent"
            }

            Column{

                TitleBar {
                    id: titleBar
                    objectName: 'titleBar'

                    width: mainWindow.width - leftSideBar.width
                    height: 25
                    iconWidth: titleBar.height
                    iconHeight: titleBar.height
                    color: "transparent"
                    windowFlag: true
                }

                WebEngineViewPage {
                    id: webEngineViewPage

                    objectName: 'webEngineViewPage'

                    width: mainWindow.width - leftSideBar.width
                    height: mainWindow.height - titleBar.height - playBottomBar.height

                    url: "http://10.0.0.153:8093/"
                    Behavior on opacity {
                        NumberAnimation { duration: 500 }
                    }
                }
            }
        }

        PlayBottomBar {
            id: playBottomBar
            // playerDuration: player.duration
            // playerPosition: player.position
            width: mainWindow.width
            height: 100

            color: "#282F3F"
        }
    }

    MainWindowController{
        id: mainWindowController
        mainWindow: mainWindow
        bgImage: bgImage
        titleBar: titleBar
        leftSideBar: leftSideBar
        webEngineViewPage: webEngineViewPage
        playBottomBar: playBottomBar
    }
}
