import QtQuick 2.3

Item {
    property var artistView

    Connections {
        target: artistView
        onPlay: {
            MusicManageWorker.playArtist(name);
        }

        onClicked:{
            SignalManager.jumpToLocalDetailArtist(name);
        }

        onRightClicked:{
            MenuWorker.artistMenuShow(name);
        }
    }
}