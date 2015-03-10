import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 1.3
import QtWebEngine 1.0
import QtMultimedia 5.0
import DMusic 1.0
import "../DMusicWidgets"


Rectangle {

    id: mainWindow

    property var views: ['WebMusic360Page', 'MusicManagerPage', 'PlayListPage', 'DownloadPage']
    property var titleBar: titleBar
    property var playBottomBar: playBottomBar
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
                width: 56
                height: mainWindow.height - playBottomBar.height
                iconWidth: leftSideBar.width
                iconHeight: leftSideBar.width
                color: "transparent"
            }

            Column{

                TitleBar {
                    id: titleBar
                    objectName: 'mainTitleBar'

                    width: mainWindow.width - leftSideBar.width
                    height: 25
                    iconWidth: titleBar.height
                    iconHeight: titleBar.height
                    color: "transparent"
                    windowFlag: true
                }

                DStackView {

                    id: stackViews
                    objectName: 'stackViews'
                    width: mainWindow.width - leftSideBar.width
                    height: mainWindow.height - titleBar.height - playBottomBar.height
                    currentIndex: 0

                    WebEngineViewPage {
                        id: webEngineViewPage

                        objectName: 'webEngineViewPage'

                        width: mainWindow.width - leftSideBar.width
                        height: mainWindow.height - titleBar.height - playBottomBar.height

                        url: "http://music.haosou.com/_linuxdeepin/"
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
            width: mainWindow.width
            height: 90

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
