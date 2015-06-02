import QtQuick 2.3
import QtQml.Models 2.1

Flickable {
    id: flickable
    property int currentIndex
    property ObjectModel objectModel
    property int spacing: 0

    contentWidth: col.childrenRect.width
    contentHeight: col.childrenRect.height
    flickableDirection: Flickable.VerticalFlick
    clip: true

    Behavior on contentY {
        NumberAnimation { easing.type: Easing.InOutCubic; duration: 300 }
    }

    function scrollbyIndex(index) {
        var visibleChildren = col.visibleChildren
        contentY = visibleChildren[index].y
    }

    onContentYChanged: {
        if (!flicking) return

        if(atYEnd) {
            flickable.currentIndex = col.visibleChildren.length - 1;
        } else {
            var currentTopItem = col.childAt(50, contentY)
            var visibleChildren = col.visibleChildren
            for (var i = 0; i < visibleChildren.length; i++) {
                if (visibleChildren[i] === currentTopItem) {
                    flickable.currentIndex = i;
                }
            }
        }
    }

    Column {
        id: col
        width: flickable.width
        height: flickable.height
        spacing: flickable.spacing
    }

    onObjectModelChanged: {
        col.children = objectModel.children
    }
}