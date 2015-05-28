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
                width: 202
                height: 22
                radius: 2
                border.width: 1
                border.color: "lightgray"
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

            OnlinePage {
                anchors.fill: parent
            }

            LocalPage {
                anchors.fill: parent
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 30
        width: parent.width

        border.width: 1
        border.color: 'lightgray'

        SearchInput {
            id: searchInput
            anchors.margins: 1
            anchors.fill: parent
        }
    }


    SearchController {
        searchPage: root
        searchInput: searchInput
    }
}