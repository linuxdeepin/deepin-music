import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: mediaItem

    width: 300
    height: 100


    signal name()
    
    Rectangle {
        anchors.fill: parent
        radius: 10
        color: 'white'
    }

    DCloseButton{
        id: closeButton
        width: 25
        height: 25

        // onClicked:{
        //     root.closed()
        // }
    }

    Component.onCompleted: {
        print(SignalManager)
    }
}