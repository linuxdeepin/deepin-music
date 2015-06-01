import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets/LeftSideBar"

Rectangle {

    id: root
    property int iconWidth
    property int iconHeight
    property alias logoButton: logoButton
    property alias web360Button: webMusic360Button
    property alias managerButton: musciManagerButton
    property alias playListButton: playListButton
    property alias downloadButton: downloadButton
    property alias searchButton: searchButton

    signal swicthViewByID(string viewID)
    signal searchButtonClicked()

    focus: false

    Rectangle {
        id: topBox
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: bottomBox.top

        height: parent.height - bottomBox.height

        Column {
            anchors.fill: parent
            spacing: 8

            DLogoButton{
                id: logoButton
                hoverEnabled: false
                width: root.iconWidth
                height: root.iconHeight
            }

            DOnlineButton{
                id: webMusic360Button
                property var viewID: 'WebMusic360Page' 
                width: root.iconWidth
                height: root.iconHeight
                onClicked:{
                    root.swicthViewByID(viewID);
                }
            }

            DLocalButton{
                id: musciManagerButton
                property var viewID: 'MusicManagerPage'
                width: root.iconWidth
                height: root.iconHeight

                onClicked:{
                    root.swicthViewByID(viewID);
                }
            }

            DPlaylistButton{
                id: playListButton
                property var viewID: 'PlayListPage'
                width: root.iconWidth
                height: root.iconHeight
                onClicked:{
                    root.swicthViewByID(viewID);
                }
            }
        }
    }

    Rectangle {
        id: bottomBox
        anchors.left: parent.left
        anchors.top: topBox.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 91
        Column {
            anchors.fill: parent
            anchors.bottomMargin: 7
            DDownloadButton{
                id: downloadButton
                property var viewID: 'DownloadPage'
                width: root.iconWidth
                height: 42

                onClicked:{
                    root.swicthViewByID(viewID);
                }
            }

            DSearchButton{
                id: searchButton
                property var viewID: 'SearchPage'
                width: root.iconWidth
                height: 42

                onClicked:{
                    root.searchButtonClicked();
                }
            }
        }
    }
}