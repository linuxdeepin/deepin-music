import QtQuick 2.3

Item {
    property var toolBar
    property var barLoader

    Connections {
        target: toolBar
        onLockedChanged: {
            if (!locked){
                barLoader.source = './UnlockBar.qml';
            }
        }

        onHoveredChanged:{
            if (locked && hovered) {
                barLoader.setSource('./LockBar.qml', {'toolBar': toolBar})
            }else if (locked && !hovered){
                barLoader.setSource('./Empty.qml')
            }
        }
    }


    Component.onCompleted: {

    }
}