import QtQuick 2.3
import QtGraphicalEffects 1.0

Rectangle {
    id: root
    color: Qt.rgba(0, 0, 0, 0)

    property var contentItem

    width: contentItem.width + contentItem.margins * 2
    height: contentItem.height + contentItem.margins * 2
    children: [effect, container]

    RectangularGlow {
        id: effect
        anchors.fill: container
        glowRadius: 2
        spread: 0.1
        color: Qt.rgba(0, 0, 0, 0.2)
        cornerRadius: contentItem.radius + glowRadius
    }

    Rectangle {
        id: container
        anchors.fill: parent
        anchors.margins: 2
        radius: 4
        color: 'transparent'
        children: [contentItem]
    }
}