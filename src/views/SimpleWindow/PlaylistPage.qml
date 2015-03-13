import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {

    id: playlistPage

    property var model: null
    property var currentIndex: -1
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
        model: playlistPage.model
        delegate: musicDelegate
        currentIndex: playlistPage.currentIndex
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
    }
}
