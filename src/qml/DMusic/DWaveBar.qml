import QtQuick 2.3

Rectangle {
    id: root
    property var itemWidth: 3
    property var itemHeight: 20
    property var active: true
    property var count: 4

    width: count * root.itemWidth + 4
    height: root.itemHeight
    anchors.centerIn: parent
    color: "transparent"

    visible: root.active

    Row {
        spacing: 1
        Rectangle {
            width: root.itemWidth
            height: root.itemHeight - y
            color: "green"
            NumberAnimation on y { 
                loops: Animation.Infinite
                running: root.active
                from :height
                to: 0
                duration: 600 
            }
        }
        Rectangle {
            width: root.itemWidth
            height: root.itemHeight - y
            color: "green"
            NumberAnimation on y { 
                loops: Animation.Infinite
                running: root.active
                from : height
                to: 0
                duration: 800 
            }
        }
        Rectangle {
            width: root.itemWidth
            height: root.itemHeight - y
            color: "green"
            NumberAnimation on y { 
                loops: Animation.Infinite
                running: root.active
                from : height
                to: 0
                duration: 400 
            }
        }
        // Rectangle {
        //     width: root.itemWidth
        //     height: root.itemHeight - y
        //     color: "green"
        //     NumberAnimation on y { 
        //         loops: Animation.Infinite
        //         running: root.active
        //         from : height
        //         to: 0
        //         duration: 1000 
        //     }
        // }
        Rectangle {
            width: root.itemWidth
            height: root.itemHeight - y
            color: "green"
            NumberAnimation on y { 
                loops: Animation.Infinite
                running: root.active
                from :height
                to: 0
                duration: 600 
            }
        }
    }
}