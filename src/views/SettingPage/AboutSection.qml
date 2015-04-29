import QtQuick 2.3

Rectangle {
    id: root

    property string subject

    Column {
        id: layout
        anchors.fill: parent
        anchors.rightMargin: 40
        
        Rectangle {
            id: header
            width: layout.width
            height: 20
            Text {
                anchors.fill: parent
                text: root.subject
            }
        }

        Rectangle {
            id: seprator
            width: layout.width
            height: 1
            color: "lightgray"
        }

        Rectangle {
            id: container
            width: layout.width
            height: root.height - header.height - seprator.height
            Rectangle {
                anchors.fill: parent
                anchors.leftMargin: 40
                anchors.topMargin: 10
                anchors.rightMargin: 40
                anchors.bottomMargin: 40
                color: "gray"
            }
        }
    }
}