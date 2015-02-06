import QtQuick 2.0

Rectangle {
    id: root
    width: 800; height: 600
    color: "#c0c0c0"

    Column {

        

        Column {
            // anchors.horizontalCenter: parent.horizontalCenter
            // anchors.verticalCenter: parent.verticalCenter

            spacing: 5

            Rectangle { color: "lightblue"; radius: 10.0
                        width: 300; height: 50
                        Text { anchors.centerIn: parent
                               font.pointSize: 24; text: "Books" } }
            Rectangle { color: "gold"; radius: 10.0
                        width: 300; height: 50
                        Text { anchors.centerIn: parent
                               font.pointSize: 24; text: "Music" } }
            Rectangle { color: "lightgreen"; radius: 10.0
                        width: 300; height: 50
                        Text { anchors.centerIn: parent
                               font.pointSize: 24; text: "Movies" } }
        }

        Rectangle{
            width: root.width
            height:100
            color: "#345678"
        }
    }

}