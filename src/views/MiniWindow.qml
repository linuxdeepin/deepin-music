import QtQuick 2.0

Rectangle {
	id: miniWindow
    width: 150
    height: 150

    BorderImage {
        id: bgImage
        objectName: 'bgImage'

        anchors.fill: miniWindow
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }
}