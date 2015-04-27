import QtQuick 2.3
import QtQml.Models 2.1
import DMusic 1.0

Rectangle{
    id: root
    anchors.fill: parent

    property var count
    property int currentIndex

    Row {
        Rectangle {
            id: navgationBar
            width: 100
            height: root.height

            Component{
                id: delegateItem
                Rectangle {
                    id: delegateRect
                    width: parent.width
                    height: 24
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

        ObjectModel {
            id: itemModel

            Rectangle {
                width: view.width; height: view.height - 200
                color: "#FFFEF0"
                Text { text: "Page 1"; font.bold: true; anchors.centerIn: parent }

                Component.onDestruction: if (printDestruction) print("destroyed 1")
            }
            Rectangle {
                width: view.width; height: view.height - 300
                color: "#F0FFF7"
                Text { text: "Page 2"; font.bold: true; anchors.centerIn: parent }

                Component.onDestruction: if (printDestruction) print("destroyed 2")
            }
            Rectangle {
                width: view.width; height: view.height - 400
                color: "#F4F0FF"
                Text { text: "Page 3"; font.bold: true; anchors.centerIn: parent }

                Component.onDestruction: if (printDestruction) print("destroyed 3")
            }

            Rectangle {
                width: view.width; height: view.height
                color: "#F4a3d2"
                Text { text: "Page 4"; font.bold: true; anchors.centerIn: parent }

                Component.onDestruction: if (printDestruction) print("destroyed 3")
            }
        }

        ListView {
            id: view
            width: root.width - navgationBar.width
            height: root.height
            model: itemModel
            clip: true
            highlightMoveDuration: 1
            // preferredHighlightBegin: 0
            // preferredHighlightEnd: 0
            // displayMarginBeginning: 0
            // displayMarginEnd: 0
            highlightRangeMode: ListView.StrictlyEnforceRange
            // highlightMoveVelocity: 2000
            snapMode: ListView.SnapOneItem; flickDeceleration: 2000
            currentIndex: root.currentIndex
            onCurrentIndexChanged: root.currentIndex = currentIndex

        }
    }
}
