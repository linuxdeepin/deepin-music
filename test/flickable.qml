import QtQuick 2.4


Rectangle{
    id: root
    width: 800
    height:500

    property var count
    property int currentIndex

    Row {
        Rectangle {
            id: navgationBar
            width: 200
            height: root.height

            Component{
                id: delegateItem
                Rectangle {
                    id: delegateRect
                    width: parent.width
                    height: 50
                    Text {
                        anchors.centerIn: parent
                        text: index
                    }
                    states: [
                        State {
                            name: "Current"
                            when: delegateRect.ListView.isCurrentItem
                            PropertyChanges { target: delegateRect; color: "#4ba3fb"}
                        }
                    ]
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.currentIndex  = index
                        }
                    }
                }
            }

            ListView {
                id: navgationListView
                width: navgationBar.width
                height: navgationListView.count * 50
                snapMode: ListView.SnapToItem
                currentIndex: root.currentIndex
                onCurrentIndexChanged: root.currentIndex = currentIndex
                focus: true
                model: 4
                delegate: delegateItem
            }
        }

        ListView {
            id: listView
            width: root.width - navgationBar.width
            height: root.height
            snapMode: ListView.SnapToItem
            currentIndex: root.currentIndex
            onCurrentIndexChanged: root.currentIndex = currentIndex
            highlightMoveVelocity: 2000
            highlightRangeMode: ListView.StrictlyEnforceRange
            model: 4
            delegate: Rectangle {
                width: listView.width
                height: listView.height

                DStackView {
                    id: stakview
                    anchors.fill: parent
                    currentIndex: index

                    Rectangle {
                        id: page1
                        anchors.fill: parent
                        color: "red"
                        Text {
                            text: 'Page1'
                        }

                        Component.onDestruction: {
                            print('page1 onDestruction')
                        }
                    }

                    Rectangle {
                        id: page2
                        anchors.fill: parent
                        color: "green"

                        Component.onDestruction: {
                            print('page2 onDestruction')
                        }
                    }

                    Rectangle {
                        id: page3
                        anchors.fill: parent
                        color: "white"

                        Component.onDestruction: {
                            print('page3 onDestruction')
                        }
                    }

                    Rectangle {
                        id: page4
                        anchors.fill: parent
                        color: "blue"

                        Component.onDestruction: {
                            print('page4 onDestruction')
                        }
                    }
                }
            }
        }
    }
}
