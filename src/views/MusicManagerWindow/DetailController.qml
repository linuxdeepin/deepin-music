import QtQuick 2.3

Item {
    property var detailPage
    property var detailImageItem
    property var detailView
    property var songListModel

    function playMusicByUrl(url) {
        MusicManageWorker.playSong(url);
        MediaPlayer.playLocalMedia(url);
    }

    Connections {
        target: detailView

        onPlayMusicByUrl: {
            playMusicByUrl(url);
        }
    }

    Connections {
        target: detailImageItem
        onPlay: {
            if(detailPage.type == 'Artist'){
                var name = detailPage.model.name;
                MusicManageWorker.playArtist(name);
            }
            else if (detailPage.type == 'Album'){
                var name = detailPage.model.name;
                MusicManageWorker.playAlbum(name);
            }
        } 
    }
}