import QtQuick 2.3
import QtQml.Models 2.1
import DMusic 1.0

Rectangle{
    id: root
    anchors.fill: parent

    property var count
    property int currentIndex


    ListModel {
        id: navgationModel

        ListElement {
            name: "基础设置"
        }
        ListElement {
            name: "快捷键"
        }
        ListElement {
            name: "桌面歌词"
        }
        ListElement {
            name: "下载"
        }
        ListElement {
            name: "关于"
        }
    }

    Row {
        anchors.fill: parent
        anchors.topMargin: 20
        Rectangle {
            id: navgationBar
            width: 120
            height: root.height
            Component{
                id: delegateItem
                Rectangle {
                    id: delegateRect
                    width: parent.width
                    height: 24
                    Text {
                        id: navText
                        anchors.fill: parent
                        anchors.leftMargin: 24
                        color: "#3a3a3a"
                        horizontalAlignment: Text.AlignLeft
                        text: name
                    }
                    states: [
                        State {
                            name: "Current"
                            when: delegateRect.ListView.isCurrentItem
                            PropertyChanges { target: navText; color: "#4ba3fb"}
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
                model: navgationModel
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
            Rectangle {
                width: view.width; height: view.height
                color: "#F411d2"
                Text { text: "Page 5"; font.bold: true; anchors.centerIn: parent }

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
            // interactive: false
            highlightRangeMode: ListView.StrictlyEnforceRange
            snapMode: ListView.SnapOneItem
            currentIndex: root.currentIndex
            onCurrentIndexChanged: root.currentIndex = currentIndex

        }
    }
}
