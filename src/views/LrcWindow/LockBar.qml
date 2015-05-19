import QtQuick 2.3
import QtQuick.Window 2.2
import DMusic 1.0
import "../DMusicWidgets/LrcWindow"


Rectangle {
    id: lockedBar

    property var toolBar

    anchors.fill: parent
    color: "transparent"

    DLrcDesktopLockButton {
        id: desktopLockButton
        anchors.centerIn: parent
        width: 32
        height: 32
        onClicked: {
            toolBar.locked = false;
            SignalManager.unLocked();                
        }
    }
}
