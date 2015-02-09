import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtWebEngine 1.0
import DMusic 1.0
import "Utils.js" as Utils

Rectangle {

    id: root
    
    property var views: ['WebMusic360Page', 'MusicManagerPage', 'PlayListPage', 'DownloadPage']
    color: "gray"

    Column{

        Row{

            LeftSideBar {
                id: leftSideBar
                width: 60
                height: root.height - buttonBar.height
                iconWidth: leftSideBar.width
                iconHeight: leftSideBar.width
                color: "transparent"
            }

            Column{

                TitleBar {
                    id: titleBar
                    width: root.width - leftSideBar.width
                    height: 25
                    iconWidth: titleBar.height
                    iconHeight: titleBar.height
                    color: "transparent"
                }

                DStackView {
                    id: stackViews
                    width: root.width - leftSideBar.width
                    height: root.height - titleBar.height - buttonBar.height
                    current: 0
                    WebMusic360Page {
                        id: webMusic360Page
                        width: stackViews.width
                        height: stackViews.height
                    }

                    MusicManagerPage {
                        id: musicManagerPage
                        width: stackViews.width
                        height: stackViews.height
                    }

                    PlayListPage {
                        id: palyListPage
                        width: stackViews.width
                        height: stackViews.height
                    }

                    DownloadPage {
                        id: downPage
                        width: stackViews.width
                        height: stackViews.height
                    }
                }
            }
        }

        Rectangle {
            id: buttonBar
            width: root.width
            height: 100
            color: "#345678"
        }
    }

    Connections {
        target: leftSideBar
        onSwicthViewByID: {
            var index =  root.views.indexOf(viewID);
            stackViews.setCurrentIndex(index);
        }
    }
}
