import QtQuick 2.4
import DMusic 1.0

GridView {
    id: grid
    anchors.fill: parent
    clip: true

    cellWidth: 108 + 56
    cellHeight: 108 + 56

    highlightMoveDuration: 1
    boundsBehavior: Flickable.StopAtBounds
    snapMode: GridView.SnapToRow 
    focus: true

    DScrollBar {
        flickable: parent
        inactiveColor: 'black'
    }

    onModelChanged:{
        positionViewAtEnd()
    }
}
