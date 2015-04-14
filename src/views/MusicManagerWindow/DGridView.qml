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

    signal play(string name, int index)
    signal clicked(string name, int index)
    signal rightClicked(string name, int index)

    DScrollBar {
        flickable: parent
        inactiveColor: 'black'
    }

    // onModelChanged:{
    //     positionViewAtEnd()
    // }
}
