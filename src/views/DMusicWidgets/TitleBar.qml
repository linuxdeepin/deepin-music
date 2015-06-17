import QtQuick 2.3
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.1
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
    signal closed()

    Row {
        anchors.fill: parent

        Rectangle{
            width: root.width - 4 * root.iconWidth
            height: root.iconHeight
            color: "transparent"
        }

        DModeSwitchButton{
            id: modeButton
            windowFlag: root.windowFlag
            width: root.iconWidth
            height: root.iconHeight

            onClicked:{
                if (windowFlag){
                    root.simpleWindowShowed();
                }else{
                    root.mainWindowShowed();
                }
            }
        }

        DMenuButton{
            id: menuButton
            width: root.iconWidth
            height: root.iconHeight

            onClicked:{
                root.menuShowed();
            }
        }

        DMinimizeButton{
            id: minButton
            width: root.iconWidth
            height: root.iconHeight

            onClicked:{
                root.showMinimized()
            }
        }

        DCloseButton{
            id: closeButton
            width: root.iconWidth
            height: root.iconHeight

            onClicked:{
                root.closed()
            }
        }
    }
}
