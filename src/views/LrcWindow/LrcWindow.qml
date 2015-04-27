import QtQuick 2.3
import QtQuick.Window 2.2
import DMusic 1.0

DMovableWindow {

    x: 200
    y: Screen.desktopAvailableHeight - 200
    width: Screen.desktopAvailableWidth - 400
    height: 100
    color: "#000000"
    opacity: 0.2

    Rectangle {
        id: toolBar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        height: 40
        // opacity: 0.2
        color: 'transparent'

    }

    Rectangle {
        id: lrcBox
        anchors.left: parent.left
        anchors.top: toolBar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        anchors.leftMargin: 50
        anchors.rightMargin: 50

        color: 'red'
        // opacity: 0.2

        Canvas {
            id: canvas
            anchors.fill: parent
            antialiasing: true
            onPaint:{
                var ctx = getContext("2d")
                ctx.font = "font-size: 100px"
                ctx.strokeStyle = 'red'
                ctx.fillText('sadfffffffffffffffffffffffffffffffff', 10, 10)
            }
        }

        // onDprogressChanged:{
        //     canvas.requestPaint();
        // }
    }
}
