import QtQuick 2.4

Item {
    property var folderView

    Connections {
        target: folderView
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