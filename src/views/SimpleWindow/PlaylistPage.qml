import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {

    id: playlistPage
    property var playlistView: playlistView
    
    anchors.fill: parent

    color: "transparent"

    Component {
        id: musicDelegate
        MusicDelegate {}
    }

    function getModel(){
        var playlist = MediaPlayer.playlist;
        if (playlist){
            var name = playlist.name
            var model = eval('Playlist_' + Qt.md5(name));
            return model;
        }else{
            return EmptyModel;
        }
    }

    ListView {
        id: playlistView
        anchors.fill: parent
        delegate: musicDelegate
        highlightMoveDuration: 1
        focus: true
        spacing: 3
        displayMarginBeginning: 0
        displayMarginEnd: 0
        model: songListModel
        snapMode:ListView.SnapToItem

        signal playMusicByUrl(string url)

        DScrollBar {
            flickable: parent
            inactiveColor: 'black'
        }

        Component.onCompleted: positionViewAtIndex(currentIndex, ListView.Center)
    }

    DListModel {
        id: songListModel
        pymodel: getModel()
    }
}
