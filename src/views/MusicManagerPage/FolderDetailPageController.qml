import QtQuick 2.3

Item {
    property var folderView

    Binding {
        target: folderView
        property: 'currentIndex'
        value: {
            var model = folderView.model
            for(var i=0; i<folderView.count; i++){
                if (model.get(i).url == MediaPlayer.url){
                    return i;
                }
            }
            return -1;
        }
    }

    Connections {
        target: folderView

        onPlayMusicByUrl: {
            SignalManager.playMusicByLocalUrl(url);
        }

        onMenuShowed:{
            MenuWorker.songMenuShow('DetailSubSongs', url);
        }
    }
}