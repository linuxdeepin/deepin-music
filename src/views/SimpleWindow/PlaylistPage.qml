import QtQuick 2.0
import Deepin.Widgets 1.0

Rectangle {
    
    anchors.fill: parent

    color: "transparent"


    Component {
        id: contactDelegate
        Rectangle {
            id: mediaItem
            width: parent.width; height: 40

            color: "transparent"


            Text { 
                anchors.centerIn: parent
                text: '<b>Name:</b> ' + index
                color: "gray"
            }


            states: State {
                name: "Current"
                when: mediaItem.ListView.isCurrentItem
                PropertyChanges { target: mediaItem; color: "green" }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    mediaItem.ListView.view.currentIndex = index
                }
            }
        }
    }

    ListView {
        anchors.fill: parent
        model: 100
        delegate: contactDelegate
        // highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
        focus: true

        DScrollBar {
            flickable: parent
            inactiveColor: 'black'
        }
    }
}