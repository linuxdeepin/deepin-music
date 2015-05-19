import QtQuick 2.3
import QtQuick.Window 2.2

Window {
    id: miniWindow

    property bool hovered: false
    flags: Qt.FramelessWindowHint | Qt.Pop| Qt.WindowStaysOnTopHint

    color: miniWindow.hovered? Qt.rgba(0, 0, 0, 0.2) : Qt.rgba(0, 0, 0, 0)

    MouseArea {
        property int dragStartX
        property int dragStartY
        property int windowLastX
        property int windowLastY

        anchors.fill: parent
        propagateComposedEvents: true
        hoverEnabled: true

        onEntered: {
            miniWindow.hovered = true
        }
        onExited:{
            miniWindow.hovered = false
        }

        onPressed: { 
            var pos = WindowManageWorker.cursorPos
            
            windowLastX = miniWindow.x
            windowLastY = miniWindow.y
            dragStartX = pos.x
            dragStartY = pos.y
        }

        onPositionChanged: {
            if (pressed) {
                var pos = WindowManageWorker.cursorPos
                miniWindow.x = (windowLastX + pos.x - dragStartX)
                miniWindow.y = (windowLastY + pos.y - dragStartY)
                windowLastX = miniWindow.x
                windowLastY = miniWindow.y
                dragStartX = pos.x
                dragStartY = pos.y
            }
        }
    }
}