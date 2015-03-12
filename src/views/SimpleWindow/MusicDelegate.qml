import QtQuick 2.4
import "../DMusicWidgets"

Rectangle {
    id: mediaItem

    property var playButton: playButton

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

            Rectangle {
                id: waveBar

                property var active: false

                anchors.centerIn: parent
                width: 30
                height: 40

                visible: false

                color: "transparent"

                Row {
                    anchors.centerIn: parent
                    spacing: 2
                    Rectangle {
                        width: (waveBar.width - 3 * 2) / 4
                        height: waveBar.height - y
                        color: "green"

                        NumberAnimation on height { 
                            loops: Animation.Infinite
                            running: waveBar.active
                            from :waveBar.height
                            to: 0
                            duration: 1000 
                        }
                    }
                    Rectangle{
                        width: (waveBar.width - 3 * 2) / 4
                        height: waveBar.height - y
                        color: "green"
                        NumberAnimation on height { 
                            loops: Animation.Infinite
                            running: waveBar.active
                            from :waveBar.height
                            to: 0
                            duration: 1200 
                        }
                    }
                    Rectangle{
                        width: (waveBar.width - 3 * 2) / 4
                        height: waveBar.height - y
                        color: "green"
                        NumberAnimation on height { 
                            loops: Animation.Infinite
                            running: waveBar.active
                            from :waveBar.height
                            to: 0
                            duration: 2000 
                        }
                    }
                    Rectangle{
                        width: (waveBar.width - 3 * 2) / 4
                        height: waveBar.height - y
                        color: "green"
                        NumberAnimation on height { 
                            loops: Animation.Infinite
                            running: waveBar.active
                            from :waveBar.height
                            to: 0
                            duration: 1600 
                        }
                    }
                }
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
                verticalAlignment: Text.AlignVCenter
                text: mediaItem.ListView.view.model[index].title
                color: "#131313"
                font.pixelSize: 14
            }

            Text {
                id: artistText
                width: mediaItem.width - playButton.width
                height: mediaItem.height / 2
                verticalAlignment: Text.AlignVCenter
                text: mediaItem.ListView.view.model[index].artist
                color: "#919191"
                font.pixelSize: 13
            }
        }
    }


    states: State {
        name: "Current"
        when: mediaItem.ListView.isCurrentItem
        PropertyChanges { target: waveBar; active: true ; visible: true}
        PropertyChanges { target: musicText; color: "#4ba3fb" ; font.pixelSize: 16}
        PropertyChanges { target: artistText; color: "#4ba3fb" ; font.pixelSize: 14}
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onEntered: {
            if (mediaItem.ListView.view.currentIndex != index){
                playButton.visible = true
            }
        }
        onExited: {
            playButton.visible = false

        }
        onClicked: {
            if (mouse.button == Qt.LeftButton){
                mediaItem.ListView.view.currentIndex = index;
                playButton.visible = false
            }
            else if (mouse.button == Qt.RightButton){
                mediaItem.menuShowed(index);
            }
        }
    }
}