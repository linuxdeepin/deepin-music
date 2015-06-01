import QtQuick 2.3
import DMusic 1.0

TextInput  {
    id: root
    anchors.left: parent.left
    anchors.verticalCenter: parent.verticalCenter
    color: "#868686"
    font.pixelSize: 12
    clip: true
    activeFocusOnTab: true
    activeFocusOnPress: true
    cursorVisible: true
    selectByMouse: true
    selectionColor: 'green'
    verticalAlignment: TextInput.AlignVCenter
    text: ''
    // readOnly: true
    // maximumLength: 10
    // wrapMode: TextInput.WordWrap
}