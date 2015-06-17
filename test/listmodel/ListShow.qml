import QtQuick 2.3

Rectangle {
    id: root

    property var count: 100
    width: 180; height: 200


    DListModel {
        id: listModel
        pymodel: myListModel
    }

    Component {
        id: contactDelegate
        Rectangle {
            id: mediaItem
            width: 180; height: 40
            Column {
                Text { text: '<b>Name:</b> ' + name}
                Text { text: '<b>Number:</b> '+ count }
            }

            states: State {
                name: "Current"
                when: mediaItem.ListView.isCurrentItem
                PropertyChanges { target: mediaItem; color: "green" }
            }

            MouseArea {
                anchors.fill: parent
                onEntered: {}
                onExited: {}
                onClicked: {
                    var item = {
                        'name': 'aaaa1111',
                        'count': 0,
                        'cover': '/dsdssd/dssdds0',
                        'songs': {'song': 'dsdsdsdsddssssssss'}
                    }
                    mediaItem.ListView.view.model.setSignal(index, item)
                    mediaItem.ListView.view.model.dremove(0);
                    mediaItem.ListView.view.model.dinsert(0, item)
                    // print(mediaItem.ListView.view.model.count, '-----')
                    // mediaItem.ListView.view.model.dremove(0);
                    // mediaItem.ListView.view.model.removeSignal(index)
                }
            }
        }
    }

    ListView {
        id: list
        anchors.fill: parent
        model: listModel
        delegate: contactDelegate
        highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
        focus: true
    }
}