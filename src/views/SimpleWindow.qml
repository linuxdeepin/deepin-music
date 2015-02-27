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
            height: simpleWindow.height - titleBar.height - bottomBar.height
            color: "gray"
        }

        Rectangle{
        	id: bottomBar
            objectName: 'bottomBar'
            width: simpleWindow.width
            height: 200
            color: "green"
        }
    }

    SimpleController{
    	bgImage: bgImage
    	titleBar: titleBar
    }
}