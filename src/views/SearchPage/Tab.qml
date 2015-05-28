import QtQuick 2.3
import QtQuick.Controls 1.0

Rectangle{
    id: root
    property string text
    property bool disabled: false
    property bool hoverEnabled: false
    property var normal_color: 'white'
    property var check_color: '#0AB9E2'

    signal hovered
    signal clicked
    signal pressed
    signal released
    signal exited

    MouseArea {

        id: mouseArea

        anchors.fill: parent
        enabled: !disabled
        hoverEnabled: parent.hoverEnabled
        onEntered: {
            parent.hovered();
            // parent.state = "hovered"
        }
        onExited: {
            parent.exited()
            // parent.state = "normal"
        }

        onPressed:{
            parent.pressed()
            // parent.state = "pressed"
        }

        onReleased:{
            parent.released()
            // parent.state = "normal"
        }

        onClicked: {
            parent.clicked();
            root.ListView.view.currentIndex = index;
        }
    }

    Text {
        id: label
        anchors.centerIn: parent
        color: "green"
        text: root.text
    }

    states:[
        State{
            when: root.ListView.isCurrentItem
            PropertyChanges {target: root; color: check_color}
            PropertyChanges {target: label; color: 'white'}
        },
        State{
            when: !root.ListView.isCurrentItem
            PropertyChanges {target: root; color: normal_color}
            PropertyChanges {target: label; color: 'gray'}
        }
    ]
}
