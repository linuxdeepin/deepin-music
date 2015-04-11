import QtQuick 2.4

Item {
    property var albumView

    Connections {
        target: albumView
        onPlay: {
            MusicManageWorker.playAlbum(name)
        }

        onClicked:{
            print(name, 'onClicked')
        }

        onRightClicked:{
            print(name)
        }
    }
}