import QtQuick 2.4

Item {
    property var artistView

    Connections {
        target: artistView
        onPlay: {
            MusicManageWorker.playArtist(name)
        }

        onClicked:{
            print(name, 'onClicked')
        }

        onRightClicked:{
            print(name)
        }
    }
}