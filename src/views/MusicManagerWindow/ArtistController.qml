import QtQuick 2.4

Item {
    property var artistView

    Connections {
        target: artistView
        onPlay: {
            print(name)
        }

        onClicked:{
            print(name)
        }

        onRightClicked:{
            print(name)
        }
    }
}