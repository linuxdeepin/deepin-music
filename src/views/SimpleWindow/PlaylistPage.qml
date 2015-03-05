import QtQuick 2.4
import Deepin.Widgets 1.0
import "../DMusicWidgets"

Rectangle {

    id: playlistPage

    property var model
    
    anchors.fill: parent

    color: "transparent"


    Component {
        id: musicDelegate
        MusicDelegate {}
    }

    ListView {
        anchors.fill: parent
        model: playlistPage.model
        delegate: musicDelegate
        focus: true
        spacing: 3
        displayMarginBeginning: 0
        displayMarginEnd: 0

        snapMode:ListView.SnapToItem

        DScrollBar {
            flickable: parent
            inactiveColor: 'black'
        }
    }
}
