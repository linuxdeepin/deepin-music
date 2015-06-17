import QtQuick 2.3

Item {
    id: stackView


    default property alias items: stack.children

    property int currentIndex: 0

    onCurrentIndexChanged: setOpacities()
    Component.onCompleted: setOpacities()

    function setOpacities() {
        for (var i = 0; i < stack.children.length; ++i) {
            // stack.children[i].opacity = (i == currentIndex ? 1 : 0)
            // stack.children[i].z = (i == currentIndex ? 1 : 0)
            stack.children[i].visible = (i == currentIndex ? true : false)
        }
    }

    function setCurrentIndex(index) {
        stackView.currentIndex = index;
    }

    function setCurrentItem(item) {
       index = stack.children.indexOf(item)
       stackView.currentIndex = index;
    }

    Item {
        id: stack
        objectName: 'stackView'
        anchors.fill: stackView
    }
}
