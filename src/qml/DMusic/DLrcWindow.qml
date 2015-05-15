import QtQuick 2.3
import QtQuick.Window 2.2

Window {
    id: root
    property bool hovered: false
    width: 500
    height: 200
    flags: Qt.FramelessWindowHint | Qt.Popup| Qt.WindowStaysOnTopHint

    color: root.hovered? Qt.rgba(0, 0, 0, 0.2) : Qt.rgba(0, 0, 0, 0)

    MouseArea {
        property int dragStartX
        property int dragStartY
        property int windowLastX
        property int windowLastY

        anchors.fill: parent
        propagateComposedEvents: true
        hoverEnabled: true

        onEntered: {
            print('+++++++++++')
            root.hovered = true
        }
        onExited:{
            root.hovered = false
        }

        // onPressed: { 
        //     var pos = WindowManageWorker.cursorPos
            
        //     windowLastX = root.x
        //     windowLastY = root.y
        //     dragStartX = pos.x
        //     dragStartY = pos.y
        // }

        // onPositionChanged: {
        //     if (pressed) {
        //         var pos = WindowManageWorker.cursorPos
        //         root.x = (windowLastX + pos.x - dragStartX)
        //         root.y = (windowLastY + pos.y - dragStartY)
        //         windowLastX = root.x
        //         windowLastY = root.y
        //         dragStartX = pos.x
        //         dragStartY = pos.y
        //     }
        // }
    }
    
    Text {
        anchors.centerIn: parent
        text: '12222222222'
    }

}