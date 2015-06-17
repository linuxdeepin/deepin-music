import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"


Rectangle {
    id: searchInputBox

    property var searchInput: searchInput

    border.width: 1
    border.color: 'lightgray'


    signal closed()

    SearchInput {
        id: searchInput
        anchors.margins: 1
        anchors.fill: parent
        focus: searchInputBox.visible
    }

    DCloseButton {
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        width: 20
        height: 20

        onClicked:{
            searchInputBox.closed();
        }
    }
    visible: false
}