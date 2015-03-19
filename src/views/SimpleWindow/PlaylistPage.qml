import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {

    id: playlistPage
    property var playlistView: playlistView
    
    anchors.fill: parent

    color: "transparent"

    Component {
        id: musicDelegate
        MusicDelegate {}
    }

    ListView {
        id: playlistView
        anchors.fill: parent
        delegate: musicDelegate
        highlightMoveDuration: 1
        focus: true
        spacing: 3
        displayMarginBeginning: 0
        displayMarginEnd: 0
        snapMode:ListView.SnapToItem

        signal changeIndex(int index)

        DScrollBar {
            flickable: parent
            inactiveColor: 'black'
        }

        Component.onCompleted: positionViewAtIndex(currentIndex, ListView.Center)
    }
}
