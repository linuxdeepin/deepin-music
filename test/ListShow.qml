import QtQuick 2.4
// import Deepin.Widgets 1.0

Rectangle {
    id: root

    property var count: 100
    width: 180; height: 200


    ListModel {
        id: fruitModel

        ListElement {
            name: "Apple"
            cost: 2.45
        }
        ListElement {
            name: "Orange"
            cost: 3.25
        }
        ListElement {
            name: "Banana"
            cost: 1.95
        }
        ListElement {
            name: "Apple"
            cost: 2.45
        }
        ListElement {
            name: "Orange"
            cost: 3.25
        }
        ListElement {
            name: "Banana"
            cost: 1.95
        }
        ListElement {
            name: "Apple"
            cost: 2.45
        }
        ListElement {
            name: "Orange"
            cost: 3.25
        }
        ListElement {
            name: "Banana"
            cost: 1.95
        }
        ListElement {
            name: "Apple"
            cost: 2.45
        }
        ListElement {
            name: "Orange"
            cost: 3.25
        }
        ListElement {
            name: "Banana"
            cost: 1.95
        }
}

    Component {
        id: contactDelegate
        Rectangle {
            id: mediaItem
            width: 180; height: 40
            Column {
                Text { text: '<b>Name:</b> ' + name}
                Text { text: '<b>Number:</b> '+ cost }
            }

            states: State {
                name: "Current"
                when: mediaItem.ListView.isCurrentItem
                PropertyChanges { target: mediaItem; color: "green" }
            }

            // MouseArea {
            //     anchors.fill: parent
            //     onClicked: {
            //         mediaItem.ListView.view.currentIndex = cost
            //     }
            // }
        }
    }

    ListView {
        id: list
        anchors.fill: parent
        model: fruitModel
        delegate: contactDelegate
        highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
        focus: true

        // DScrollBar {
        //     flickable: parent
        //     inactiveColor: 'black'
        // }
    }

    Timer{
        interval: 1000
        repeat: true
        running: true
        onTriggered:{
            print('+++')
            fruitModel.insert(0, {"name": "0", "cost": 10})
            fruitModel.append({"name": "11110", "cost": 11110})
        }
    }

    // Timer {
    //     id: timer
    //     interval: 5000
    //     repeat: true
    //     running: true
    //     onTriggered: {
    //         root.count += 10000
    //         list.model = root.count
    //         print(root.count)
    //         // if (root.count > 20 * 100){
    //         //     root.count = 100
    //         //     print('++++++++++++++')
    //         //     timer.repeat = false
    //         //     list.model = root.count * 10
    //         //     timer.stop()
    //         // }
    //     }
    // }
}