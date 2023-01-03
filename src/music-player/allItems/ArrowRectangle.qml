import QtQuick 2.11

Rectangle {
    property real rectRadius: 8
    property real triangleWidth: 10
    property real triangleHeight: 6
    property color fillColor: "#474747"

    width: 50
    height: 25
    color: "transparent"

    Column {
        width: parent.width
        height: parent.height
        Rectangle {
            id: topRect
            width: parent.width
            height: parent.height - triangleHeight
            radius: rectRadius
            color: palette.highlight
        }
        Canvas {
            id: triangleCanvas
            width: triangleWidth
            height: triangleHeight
            anchors.horizontalCenter: parent.horizontalCenter
            contextType: "2d"

            onPaint: {
                context.lineWidth = 0
                context.fillStyle = fillColor
                context.beginPath();

                context.moveTo(0, 0)
                context.lineTo(width / 2, height)
                context.lineTo(width, 0)

                /*context.moveTo(0, rectRadius)
                context.lineTo(0, height - rectRadius - triangleHeight)
                context.arcTo(0, height - rectRadius, rectRadius, height - triangleHeight, rectRadius)  //bottomLeft

                context.lineTo((width - triangleWidth) / 2, height - triangleHeight)
                context.lineTo(width / 2, height)
                context.lineTo((width + triangleWidth) / 2, height - triangleHeight)
                context.lineTo(width - rectRadius, height - triangleHeight)

                context.moveTo(42, 24)
                context.arcTo(width - rectRadius, height - triangleHeight, width, height - rectRadius - triangleHeight, rectRadius) //bottomRight
                context.moveTo(width, height - rectRadius - triangleHeight)
                context.lineTo(width, rectRadius)
                context.arcTo(width, rectRadius, width - rectRadius, 0, rectRadius) //topRight
                context.moveTo(width - rectRadius, 0)
                context.lineTo(rectRadius, 0)
                context.arcTo(rectRadius, 0, 0, rectRadius, rectRadius)*/

                context.closePath();
                context.fill()
//                context.stroke();
            }
        }
    }

    onFillColorChanged: {
        triangleCanvas.requestPaint()
    }
}
