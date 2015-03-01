import QtQuick 2.0
import QtQuick.Window 2.2
import DMusic 1.0

Window {
    id: root

    flags: Qt.FramelessWindowHint | Qt.Dialog

    MouseArea {

        anchors.fill: root

        property int dragStartX
        property int dragStartY
        property int windowLastX
        property int windowLastY

        propagateComposedEvents: true

        onPressed: { 
            var pos = WindowManageWorker.cursorPos
            
            windowLastX = root.x
            windowLastY = root.y
            dragStartX = pos.x
            dragStartY = pos.y

            print('DMoveable mouse')

            mouse.accepted = false
        }
        onPositionChanged: {
            if (pressed) {
                var pos = WindowManageWorker.cursorPos
                root.x = (windowLastX + pos.x - dragStartX)
                root.y = (windowLastY + pos.y - dragStartY)
                windowLastX = root.x
                windowLastY = root.y
                dragStartX = pos.x
                dragStartY = pos.y
            }
            // mouse.accepted = false
        }
    }
}