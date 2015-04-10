import QtQuick 2.4

Item {
    property var folderView
    property var folderListModel

    function init() {
        initFolderModel()
        MusicManageWorker.addFolderElement.connect(folderListModel.append)
        MusicManageWorker.updateFolderElement.connect(folderListModel.setProperty)
    }

    function initFolderModel(){
        for(var i=0; i<MusicManageWorker.folders.length; i++){
            var obj = MusicManageWorker.folders[i];
            folderListModel.append(obj);
        }
    }

    Connections {
        target: folderView
        onPlay: {
            MusicManageWorker.playFolder(name)
        }

        onClicked:{
            print(name)
        }

        onRightClicked:{
            print(name)
        }
    }

    Component.onCompleted: {
        init();
    }
}