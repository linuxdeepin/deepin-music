import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets/PublicWidgets"

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
    height: 24
    color: "transparent"

    Row {

        anchors.fill: parent

        spacing: 0

        Rectangle {
            width: 28
            height: 24

            color: "transparent"

            DWaveBar {
                id: waveBar
                itemHeight: 16
                itemWidth: 3
                active: mediaItem.isWavBarActive
            }

            DPlayTipButton {
                id: playButton
                anchors.centerIn: parent
                width: 24
                height: 24
                visible: false
            }
        }

        
        Row {
            spacing: 16

            Image {
                width: 24
                height: 24
                asynchronous: true
                source: cover
            }

            Text {
                id: musicText
                width: 164
                height: mediaItem.height
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                text: title
                color: "#a0a0a0"
                font.pixelSize: 11
            }

            Text {
                id: durationText
                width: 34
                height: mediaItem.height
                verticalAlignment: Text.AlignVCenter
                text: UtilWorker.duration_to_string(duration)
                color: "#a0a0a0"
                font.pixelSize: 11
            }

            Rectangle{
                width: 4
                height: mediaItem.height
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
        }
    ]

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onEntered: {
            mediaItem.state = 'Entered';
        }
        onExited: {
            mediaItem.state = 'Exited';
        }
        onClicked: {
            if (mouse.button == Qt.RightButton){
                mediaItem.ListView.view.menuShowed(url);
            }
        }
        onDoubleClicked: {
            if (mouse.button == Qt.LeftButton){
                mediaItem.ListView.view.currentIndex = index;
                if (playButton.visible){
                    mediaItem.ListView.view.playMusicByUrl(url);
                }
                playButton.visible = false;
            }
        }
    }
}