import QtQuick 2.3
import DMusic 1.0

ListView {
    id: root
    anchors.fill: parent
    clip: true
    orientation: ListView.Horizontal
    model: ListModel {
        ListElement {
            name: "Bill Smith"
        }
        ListElement {
            name: "John Brown"
        }
    }
    delegate: Tab {
        width: root.width / root.count
        height: root.height
        text: name
        onClicked:{
            ListView.view.currentIndex = index;
        }
    }
}