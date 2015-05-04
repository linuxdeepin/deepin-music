import QtQuick 2.3
import QtGraphicalEffects 1.0


Rectangle{
    id: musicImage

    property var source
    property var color: 'gray'

    width: 60
    height: 60

    RectangularGlow {
        id: effect
        x: image.x + 3
        y: image.y + 3

        width: image.width
        height: image.height
        glowRadius: 0
        spread: 0.2
        color: musicImage.color
    }

    BorderImage {
        id: image
        asynchronous: true
        anchors.fill: musicImage
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
        source: musicImage.source
    }
}
