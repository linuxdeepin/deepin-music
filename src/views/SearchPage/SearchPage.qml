import QtQuick 2.3
import DMusic 1.0

Rectangle {
    id: root
    property int currentIndex: 0
    color: "white"

    Column {
        Rectangle {
            width: parent.width
            height: 30

            Rectangle {
                anchors.centerIn: parent
                width: tabBar.width
                height: tabBar.height
                TabBar {
                    id: tabBar
                    anchors.centerIn: parent
                    width: 200
                    height: 20
                }
            }
        }

        DStackView {
            id: stackViews
            width: root.width
            height: root.height - titleBar.height
            currentIndex: tabBar.currentIndex

            Rectangle {
                anchors.fill: parent
                Text {
                    anchors.centerIn: parent
                    text: '11111111'
                }
            }
           

            Rectangle {
                anchors.fill: parent
                Text {
                    anchors.centerIn: parent
                    text: '22222222222'
                }
            }
        }
    }

    
}