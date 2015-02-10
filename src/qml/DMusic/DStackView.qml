import QtQuick 2.0

Item {
    id: stackView


    default property alias items: stack.children

    property int current: 0

    onCurrentChanged: setOpacities()
    Component.onCompleted: setOpacities()

    function setOpacities() {
        for (var i = 0; i < stack.children.length; ++i) {
            stack.children[i].opacity = (i == current ? 1 : 0)
            stack.children[i].z = (i == current ? 10 : 0)
        }
    }

    function setCurrentIndex(index) {
        stackView.current = index;
    }

    function setCurrentItem(item) {
       index = stack.children.indexOf(item)
       stackView.current = index;
    }

    Item {
        id: stack
        objectName: 'stackView'
        anchors.fill: stackView
    }
}
