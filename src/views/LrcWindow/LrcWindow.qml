import QtQuick 2.3
import QtQuick.Window 2.2
import DMusic 1.0
import "../DMusicWidgets/LrcWindow"

Window {
    id: toolBar
    property bool locked: false
    property bool hovered: false
    property var barLoader
    property int largerHeight: 24
    property int normalHeight: 16
    flags: Qt.FramelessWindowHint | Qt.Popup| Qt.WindowStaysOnTopHint
    color: Qt.rgba(0, 0, 0, 0)

    height: 50
    width : locked ? 50: 436

    MouseArea {

        anchors.fill: parent
        hoverEnabled: true

        onEntered: {
            toolBar.hovered = true
        }
        onExited:{
            toolBar.hovered = false
        }
    }

    Loader {
        id: barLoader
        anchors.fill: parent
        source: './UnlockBar.qml'
    }

    LrcController {
        toolBar: toolBar
        barLoader: barLoader
    }
}
