import QtQuick 2.3
import QtQuick.Controls 1.0

Rectangle{
    id: root
    property string text
    property bool disabled: false
    property bool hoverEnabled: true
    property var normal_color: 'dark'
    property var hover_color: 'dark'
    property var pressed_color: 'gray'
    property var disabled_color: 'gray'


    signal hovered
    signal clicked
    signal pressed
    signal released
    signal exited

    radius: 2
    border.width: 1
    border.color: "lightgray"
    state: 'normal'

    MouseArea {

        id: mouseArea

        anchors.fill: parent
        enabled: !disabled
        hoverEnabled: parent.hoverEnabled
        onEntered: {
            parent.hovered();
            parent.state = "hovered"
        }
        onExited: {
            parent.exited()
            parent.state = "normal"
        }

        onPressed:{
            parent.pressed()
            parent.state = "pressed"
        }

        onReleased:{
            parent.released()
            parent.state = "normal"
        }

        onClicked: {
            parent.clicked()
        }
    }

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
    }

    states:[
        State{
            name: "normal"
            PropertyChanges {target: label; color: normal_color}
        },
        State{
            name: "hovered"
            PropertyChanges {target: label; color: hover_color}
        },
        State{
            name: "pressed"
            PropertyChanges {target: label; color: pressed_color}
        },
        State{
            name: "disabled"
            PropertyChanges {target: label; color: disabled_color}
        }
    ]
}
