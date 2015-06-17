import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets/SimplePlayer"

Rectangle {
    id: mediaItem

    property var playButton: playButton
    property bool isWavBarActive: {
        var playlist = MediaPlayer.playlist;
        if (playlist){
            if (playlist.url == url && MediaPlayer.playing){
                return true
            }else{
                return false
            }
        }
        else{
            return false
        }
    }

    width: parent.width
    height: 44
    color: "transparent"

    signal menuShowed(int index)

    Row {

        anchors.centerIn: parent

        Rectangle {
            width: 40
            height: 40

            color: "transparent"

            DWaveBar {
                id: waveBar
                itemHeight: 30
                itemWidth: 5
                active: mediaItem.isWavBarActive
            }

            DPlayButton {
                id: playButton
                anchors.fill: parent
                width: 40
                height: 40
                visible: false
            }

        }

        Column {
            Text {
                id: musicText
                width: mediaItem.width - playButton.width
                height: mediaItem.height / 2
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                text: mediaItem.ListView.view.model.get(index).title
                color: "#131313"
                font.pixelSize: 14
            }

            Text {
                id: artistText
                width: mediaItem.width - playButton.width
                height: mediaItem.height / 2
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                text: mediaItem.ListView.view.model.get(index).artist
                color: "#919191"
                font.pixelSize: 13
            }
        }
    }

    states: [
        State {
            name: "Entered"
            PropertyChanges { target: mediaItem; color: "lightgray";}
            PropertyChanges { target: playButton; visible: !mediaItem.isWavBarActive;}
        },
        State {
            name: "Exited"
            PropertyChanges { target: mediaItem; color: "transparent";}
            PropertyChanges { target: playButton; visible: false;}
        },
        State {
            name: "DoubleClicked"
            when: mediaItem.ListView.isCurrentItem
            PropertyChanges { target: playButton; visible: !mediaItem.isWavBarActive ;}
        },

        State {
            name: "Current"
            when: mediaItem.ListView.isCurrentItem
            PropertyChanges { target: musicText; color: "#4ba3fb" ; font.pixelSize: 16}
            PropertyChanges { target: artistText; color: "#4ba3fb" ; font.pixelSize: 14}
        }
    ]

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onEntered: {
            mediaItem.state = 'Entered'
        }
        onExited: {
            mediaItem.state = 'Exited'
        }

        onClicked:{
            if (mouse.button == Qt.RightButton){
                mediaItem.menuShowed(index);
            }
        }

        onDoubleClicked: {
            if (mouse.button == Qt.LeftButton){
                mediaItem.ListView.view.currentIndex = index;
                if (playButton.visible){
                    var url = mediaItem.ListView.view.model.get(index).url;
                    mediaItem.ListView.view.playMusicByUrl(url);
                }
            }
        }
    }
}