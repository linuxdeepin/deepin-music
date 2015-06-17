import QtQuick 2.3

Item {

    property var onlineSongsView
    property var albumSection
    property var playlistSection


    Connections {
        target: onlineSongsView
        onPlayMusicByUrl: {

            var title = onlineSongsView.model.get(index).title;
            var artist = onlineSongsView.model.get(index).artist;
            var localUrl = DownloadSongWorker.getDownloadSongByKey(artist, title);
            if (localUrl){
                SignalManager.playMusicByLocalUrl(localUrl);
                onlineSongsView.model.setProperty(index, 'url', localUrl)
            }
            else{
                var songId = onlineSongsView.model.get(index).songId;
                SignalManager.playMusicBySongIdSignal(songId);
            }
        }
    }

    Connections {
        target: albumSection.view

        onPlay: {
            var albumId = albumSection.view.model.get(index).albumId;
            SignalManager.playMusicByAlbumIdSignal(albumId);
        }
    }


    Connections {
        target: playlistSection.view

        onPlay: {
            var playAll = name;
            SignalManager.playMusicByIdsSignal(playAll);
        }
    }
}