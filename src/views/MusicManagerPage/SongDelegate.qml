import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"
import "../DMusicWidgets/PublicWidgets"

Rectangle {
    id: mediaItem

    property var mouseArea: mouseArea
    property bool isWavBarActive: {
        var playlist = MediaPlayer.playlist;
        if (playlist){
            if (playlist.url == url){
                if (MediaPlayer.playing){
                    return true
                }else{
                    return false
                }
            }
            else{
                return false
            }
        }
        else{
            return false
        }
    }
    property bool isPlayTipButtonVisible: {
        var playlist = MediaPlayer.playlist;
        if (playlist){
            if (playlist.url == url){
                return !mediaItem.isWavBarActive
            }else{
                return false
            }
        }else{
            return false
        }
    }

    property bool isFavorite:PlaylistWorker.isFavorite(url);

    function favoriteOn(songUrl) {
        if (songUrl == url){
            favoriteButton.isFavorite = true;
        }
    }

    function favoriteOff(songUrl) {
        if (songUrl == url){
            favoriteButton.isFavorite = false;
        }
    }

    width: parent.width
    height: 24
    color: "transparent"

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onEntered: {
            mediaItem.state = 'Entered';
        }

        onExited: {
            mediaItem.state = "Exited";
        }

        onClicked:{
            var url = mediaItem.ListView.view.model.get(index).url;
            if (mouse.button == Qt.RightButton){
                mediaItem.ListView.view.menuShowed(url);
            }
        }
        onDoubleClicked: {
            var url = mediaItem.ListView.view.model.get(index).url;
            if (mouse.button == Qt.LeftButton){
                mediaItem.ListView.view.currentIndex = index;
                mediaItem.ListView.view.playMusicByUrl(url);
            }
        }
    }


    Row {

        anchors.fill: parent
        spacing: 0

        Rectangle{
            id: tipRect
            width: 24
            height: 24

            color: "transparent"

            Text {
                id: indexTip
                anchors.centerIn: parent
                color: "#8a8a8a"
                font.pixelSize: 10
                text: index + 1
                visible: !mediaItem.isWavBarActive
            }

            DPlayTipButton {
                id: playButton
                anchors.centerIn: parent
                width: 24
                height: 24
                visible: false
            }

            DWaveBar {
                id: waveBar
                anchors.centerIn: parent
                itemHeight: 12
                itemWidth: 3
                active: mediaItem.isWavBarActive
            }
        }

        Row {
            height: 24
            spacing: 20
            Rectangle{
                width: 238
                height: 24
                color: "transparent"
                Text {
                    id: titleTetx
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    color: "#3a3a3a"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    text: title
                }
            }

            Rectangle{
                width: 160
                height: 24
                color: "transparent"
                Text {
                    id: artistTetx
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    color: waveBar.active? "##2ca7f8":"#8a8a8a"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    text: artist
                }
            }
            
            Rectangle{
                width: 160
                height: 24
                color: "transparent"
                Text {
                    id: albumTetx
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    color: "#8a8a8a"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    text: album
                }
            }

            Row {
                spacing: 30
                
                Rectangle{
                    width: 58
                    height: 24
                    color: "transparent"
                    Text {
                        id: durationTetx
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        color: "#8a8a8a"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        text: UtilWorker.duration_to_string(duration)
                    }
                }

                Row {

                    DFavoriteButton {
                        id: favoriteButton
                        width: 24
                        height: 24
                        isFavorite: mediaItem.isFavorite
                        onClicked:{
                            if (!favoriteButton.isFavorite){
                                SignalManager.addtoFavorite(url);
                            }else{
                                SignalManager.removeFromFavorite(url);
                            }
                        }
                    }
                }

            }

        }
    }

    states: [
        State {
            name: "Entered"
            PropertyChanges { target: mediaItem; color: "lightgray";}
            PropertyChanges { target: playButton; visible: !mediaItem.isWavBarActive;}
            PropertyChanges { target: indexTip; visible: false;}
        },
        State {
            name: "Exited"
            PropertyChanges { target: mediaItem; color: "transparent";}
            PropertyChanges { target: playButton; visible: false;}
            PropertyChanges { target: indexTip; visible: !mediaItem.isWavBarActive;}
        },
        State {
            name: "DoubleClicked"
            when: mediaItem.ListView.isCurrentItem
            PropertyChanges { target: indexTip; visible: !mediaItem.isWavBarActive ;}
            PropertyChanges { target: playButton; visible: mediaItem.isPlayTipButtonVisible}
        }
    ]

    Component.onCompleted: {
        SignalManager.addtoFavorite.connect(favoriteOn);
        SignalManager.removeFromFavorite.connect(favoriteOff);
    }
}