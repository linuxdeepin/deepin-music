import QtQuick 2.4
import "../DMusicWidgets"

Rectangle {
    id: root

    DGridView {
        id: grid
        model: 52
        delegate: ItemDelegate{}
    }
}