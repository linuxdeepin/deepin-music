import QtQuick 2.3

Item {
    property var artistView

    Connections {
        target: artistView
        onPlay: {
            MusicManageWorker.playArtist(name);
        }

        onClicked:{
            MusicManageWorker.detailArtist(name, index);
        }

        onRightClicked:{
            MenuWorker.artistMenuShow(name);
        }
    }
}