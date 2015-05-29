import QtQuick 2.3

Item {

    property var localSongsView
    property var linkTipText

    function playMusicByUrl(url) {
        MusicManageWorker.playSong(url);
        MediaPlayer.playLocalMedia(url);
    }

    function getTextByLength(s, l){
        if (s.length > l){
            return s.slice(0, l) + '...'
        }else{
            return s
        }
    }

    Binding {
        target: localSongsView
        property: 'currentIndex'
        value: {
            var model = localSongsView.model
            for(var i=0; i<localSongsView.count; i++){
                if (model.get(i).url == MediaPlayer.url){
                    return i;
                }
            }
            return -1;
        }
    }

    Binding {
        target: linkTipText
        property: 'text'
        value: {
            var startText = "没有找到与\" <a href=\"Online\" style=\"text-decoration:none;\">";
            var endText = "</a>\" 相关的音乐";
            var result = getTextByLength(SearchWorker.keyword, 20);
            return startText + result + endText;
        }
    }

    Connections {
        target: localSongsView

        onPlayMusicByUrl: {
            playMusicByUrl(url);
        }

        onMenuShowed:{
            MenuWorker.searchLocalSongShowed(url);
        }
    }
}