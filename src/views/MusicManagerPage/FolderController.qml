import QtQuick 2.3

Item {
    property var folderView
    property var folderListModel

    Connections {
        target: folderView
        onPlay: {
            MusicManageWorker.playFolder(folderListModel.get(index).name);
        }

        onClicked:{
            MusicManageWorker.detailFolder(name, index);
        }

        onRightClicked:{
            MenuWorker.folderMenuShow(folderListModel.get(index).name);
        }
    }

    Component.onCompleted: {

    }
}