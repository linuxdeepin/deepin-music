import QtQuick 2.3
import QtQml.Models 2.1

ListView {
    id: listView
    anchors.fill: parent
    clip: true
    currentIndex: 0
    focus: true
    spacing: 10
    // snapMode:ListView.SnapToItem

    DScrollBar {
        flickable: parent
        inactiveColor: 'black'
    }

    // Component.onCompleted: positionViewAtIndex(currentIndex, ListView.Center)
}