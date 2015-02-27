import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import DMusic 1.0


Rectangle {

    id: root
    property int iconWidth
    property int iconHeight
    property alias modeButton: modeButton
    property alias menuButton: menuButton
    property alias minButton: minButton
    property alias closeButton: closeButton
    property var windowFlag: true

    signal simpleWindowShowed()
    signal mainWindowShowed()
    signal showMinimized()
    signal menuShowed()

    Row {
        anchors.fill: parent

        Rectangle{
            width: root.width - 4 * root.iconWidth
            height: root.iconHeight
            color: "transparent"
        }

        DIconButton{
            id: modeButton
            width: root.iconWidth
            height: root.iconHeight
            normal_image: windowFlag ? '../skin/icons/light/appbar.layout.expand.left.png': '../skin/icons/light/appbar.layout.expand.right.png'
            hover_image: windowFlag ? '../skin/icons/light/appbar.layout.expand.left.png' : '../skin/icons/light/appbar.layout.expand.right.png'
            pressed_image: windowFlag ? '../skin/icons/dark/appbar.layout.expand.left.png' : '../skin/icons/light/appbar.layout.expand.right.png'
            disabled_image: windowFlag ? '../skin/icons/light/appbar.layout.expand.left.png' : '../skin/icons/light/appbar.layout.expand.right.png'

            onClicked:{
                if (windowFlag){
                    root.simpleWindowShowed();
                }else{
                    root.mainWindowShowed();
                }
            }
        }

        DIconButton{
            id: menuButton
            width: root.iconWidth
            height: root.iconHeight
            normal_image: '../skin/icons/light/appbar.list.png'
            hover_image: '../skin/icons/light/appbar.list.png'
            pressed_image: '../skin/icons/dark/appbar.list.png'
            disabled_image: '../skin/icons/light/appbar.list.png'

            onClicked:{
                root.menuShowed();
            }
        }

        DIconButton{
            id: minButton
            width: root.iconWidth
            height: root.iconHeight
            normal_image: '../skin/icons/light/appbar.minus.png'
            hover_image: '../skin/icons/light/appbar.minus.png'
            pressed_image: '../skin/icons/dark/appbar.minus.png'
            disabled_image: '../skin/icons/light/appbar.minus.png'

            onClicked:{
                root.showMinimized()
            }
        }

        DIconButton{
            id: closeButton
            width: root.iconWidth
            height: root.iconHeight
            normal_image: '../skin/icons/light/appbar.close.png'
            hover_image: '../skin/icons/light/appbar.close.png'
            pressed_image: '../skin/icons/dark/appbar.close.png'
            disabled_image: '../skin/icons/light/appbar.close.png'

            onClicked:{
                Qt.quit()
            }
        }
    }
}
