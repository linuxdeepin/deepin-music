import QtQuick 2.0
import DMusic 1.0

Rectangle {
    id: simpleWindow

    property var titleBar: titleBar
    property var simpleWindowController: simpleWindowController

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

    SimpleWindowController{
        id: simpleWindowController

        simpleWindow: simpleWindow
        bgImage: bgImage
        titleBar: titleBar
        playBottomBar: playBottomBar
    }
}