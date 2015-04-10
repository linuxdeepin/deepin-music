import QtQuick 2.4

Item {
    property var artistView
    property var artistListModel

    function init() {
        initArtistModel()
        MusicManageWorker.addArtistElement.connect(artistListModel.append)
        MusicManageWorker.updateArtistElement.connect(artistListModel.setProperty)
    }

    function initArtistModel(){
        for(var i=0; i<MusicManageWorker.artists.length; i++){
            var obj = MusicManageWorker.artists[i];
            artistListModel.append(obj);
        }
    }
        
    // }

    // function addElementToListModel(obj){
    //     artistListModel.append(obj)
    // }

    // function updateElement(index, key, value){
    //     artistListModel.setProperty(index, key, value);
    // }

    Binding {
        target: artistView
        property: 'model'
        value: artistListModel
    }

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

    Component.onCompleted: {
        init();
    }
}