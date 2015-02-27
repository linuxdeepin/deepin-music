import QtQuick 2.0

Rectangle {
	id: simpleWindow
    width: 300
    height: 660

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
            objectName: 'titleBar'
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
            color: "gray"
        }

        Rectangle{
        	id: playBottomBar
            objectName: 'playBottomBar'

            property var slider: slider

            width: simpleWindow.width
            height: 200
            color: "green"

            SimpleSlider {
                id: slider
                progressBarColor: '#0AB9E2'
                width: playBottomBar.width
                height: 6
            }
        }
    }

    SimpleController{
    	bgImage: bgImage
    	titleBar: titleBar
        playBottomBar: playBottomBar
    }
}