import QtQuick 2.0
import DMusic 1.0

Rectangle {
    id: simpleWindow

    property var titleBar: titleBar
    property var simpleController: simpleController

    width: 300
    height: 660

    focus: true

    color: "#282F3F"

    BorderImage {
        id: bgImage
        objectName: 'bgImage'

        anchors.fill: simpleWindow
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }
    Column{
        TitleBar {
            id: titleBar
            objectName: 'simpletitleBar'

            width: simpleWindow.width
            height: 25
            iconWidth: titleBar.height
            iconHeight: titleBar.height
            color: "transparent"
            windowFlag: false
        }

        Rectangle{
            id: mainPage
            objectName: 'mainPage'
            width: simpleWindow.width
            height: simpleWindow.height - titleBar.height - playBottomBar.height
            color: "transparent"
        }

        SimplePlayBar{
            id: playBottomBar
            objectName: 'playBottomBar'

            width: simpleWindow.width
            color: "transparent"
        }
    }

    SimpleController{
        id: simpleController
        bgImage: bgImage
        titleBar: titleBar
        playBottomBar: playBottomBar
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_F1) {
            simpleController.resetSkin();
            // event.accepted = true;
        }else if (event.key == Qt.Key_F2) {
            simpleController.setSkinByImage();
            // event.accepted = true;
        }
    }
}