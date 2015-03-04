import QtQuick 2.4
import Deepin.Widgets 1.0

Rectangle {
    width: 180; height: 200

    Component {
        id: contactDelegate
        Rectangle {
            id: mediaItem
            width: 180; height: 40
            Column {
                Text { text: '<b>Name:</b> ' + index}
                Text { text: '<b>Number:</b> ' }
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
        model: 5
        delegate: contactDelegate
        highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
        focus: true

        DScrollBar {
            flickable: parent
            inactiveColor: 'black'
        }
    }
}