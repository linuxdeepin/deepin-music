import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets/MusicManager"

Item {
    id: root

    property var source

    width: 108
    height: 108

    signal play(string name, int index)
    signal clicked(string name, int index)
    signal rightClicked(string name, int index)

    Image {
        asynchronous: true
        width: root.width
        height: root.height
        source: root.source
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons:  Qt.LeftButton | Qt.RightButton
        propagateComposedEvents: true
        hoverEnabled: true
        onEntered: {
            playButton.visible = true;
        }
        onExited: {
            playButton.visible = false;
        }
        onClicked: {
            if (mouse.button == Qt.LeftButton){
                root.clicked(item.name, index);
                mouse.accpted = true
            }else if (mouse.button == Qt.RightButton){
                root.rightClicked(item.name, index);
                mouse.accpted = true
            }
        }

        onPressed:{
            mouse.accepted = true;
        }

        onDoubleClicked:{
            mouse.accepted = false;
        }

        onPositionChanged:{
            mouse.accepted = false;
        }
    }

    DPlayButton {
        id: playButton
        anchors.centerIn: parent
        width: 40
        height: 40
        visible: false
        onHovered:{
            visible = true;
        }

        onExited: {
            visible = false;
        }
        onClicked:{
            root.play(item.name, index);
        }
    }
}