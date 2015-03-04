import QtQuick 2.4
import QtGraphicalEffects 1.0

Rectangle {
    width: 400
    height: 300
    color: "white"

    Rectangle{
        id: musicImage
        width: 60
        height: 60

        anchors.centerIn: parent

        RectangularGlow {
            id: effect
            x: image.x + 3
            y: image.y + 3

            width: image.width
            height: image.height
            glowRadius: 0
            spread: 0.2
            color: "gray"
        }

        BorderImage {
            id: image
            anchors.fill: musicImage
            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
            source: "../src/skin/images/bg2.jpg"
        }
    }
}
