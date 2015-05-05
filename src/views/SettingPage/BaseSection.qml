import QtQuick 2.3

Rectangle {
    id: root

    property string subject
    property int headerHeight: 20
    property var sepratorHeight: 1
    property color sepratorColor: "lightgray"
    property Rectangle content

    Column {
        id: layout
        anchors.fill: parent
        anchors.rightMargin: 40
        
        Rectangle {
            id: header
            width: layout.width
            height: root.headerHeight
            Text {
                anchors.fill: parent
                text: root.subject
            }
        }

        Rectangle {
            id: seprator
            width: layout.width
            height: root.sepratorHeight
            color: root.sepratorColor
        }

        Rectangle {
            id: container
            width: layout.width
            height: root.height - header.height - seprator.height
            children: content
        }
    }
}