import QtQuick 2.0
import DMusic 1.0


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
    signal globalSearched()

    Column {
        anchors.fill: parent

        DIconButton{
            id: logoButton
            hoverEnabled: false
            width: root.iconWidth
            height: root.iconHeight
            normal_image: '../skin/icons/dark/appbar.music.png'
            hover_image: '../skin/icons/dark/appbar.music.png'
            pressed_image: '../skin/icons/light/appbar.music.png'
            disabled_image: '../skin/icons/dark/appbar.music.png'
        }

        DIconButton{
            id: webMusic360Button
            property var viewID: 'WebMusic360Page' 
            width: root.iconWidth
            height: root.iconHeight
            normal_image: '../skin/icons/dark/appbar.zune.png'
            hover_image: '../skin/icons/dark/appbar.zune.png'
            pressed_image: '../skin/icons/light/appbar.zune.png'
            disabled_image: '../skin/icons/dark/appbar.zune.png'

            onClicked:{
                root.swicthViewByID(viewID);
            }
        }

        DIconButton{
            id: musciManagerButton
            property var viewID: 'MusicManagerPage'
            width: root.iconWidth
            height: root.iconHeight
            normal_image: '../skin/icons/dark/appbar.folder.png'
            hover_image: '../skin/icons/dark/appbar.folder.png'
            pressed_image: '../skin/icons/light/appbar.folder.png'
            disabled_image: '../skin/icons/dark/appbar.folder.png'

            onClicked:{
                root.swicthViewByID(viewID);
            }
        }

        DIconButton{
            id: playListButton
            property var viewID: 'PlayListPage'
            width: root.iconWidth
            height: root.iconHeight
            normal_image: '../skin/icons/dark/appbar.list.png'
            hover_image: '../skin/icons/dark/appbar.list.png'
            pressed_image: '../skin/icons/light/appbar.list.png'
            disabled_image: '../skin/icons/dark/appbar.list.png'

            onClicked:{
                root.swicthViewByID(viewID);
            }
        }

        Rectangle{
            width: root.width
            height: root.height - 6 * root.iconHeight
            color: "transparent"
        }

        DIconButton{
            id: downloadButton
            property var viewID: 'DownloadPage'
            width: root.iconWidth
            height: root.iconHeight
            normal_image: '../skin/icons/dark/appbar.download.png'
            hover_image: '../skin/icons/dark/appbar.download.png'
            pressed_image: '../skin/icons/light/appbar.download.png'
            disabled_image: '../skin/icons/dark/appbar.download.png'

            onClicked:{
                root.swicthViewByID(viewID);
            }
        }

        DIconButton{
            id: searchButton
            width: root.iconWidth
            height: root.iconHeight
            normal_image: '../skin/icons/dark/appbar.magnify.png'
            hover_image: '../skin/icons/dark/appbar.magnify.png'
            pressed_image: '../skin/icons/light/appbar.magnify.png'
            disabled_image: '../skin/icons/dark/appbar.magnify.png'

            onClicked:{
                root.globalSearched()
            }
        }
    }
}