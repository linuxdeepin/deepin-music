import QtQuick 2.3

Item {
    property var albumView

    Connections {
        target: albumView
        onPlay: {
            MusicManageWorker.playAlbum(name)
        }

        onClicked:{
            MusicManageWorker.jumpToLocalDetailAlbum(name)
        }

        onRightClicked:{
            MenuWorker.albumMenuShow(name);
        }
    }
}