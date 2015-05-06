import QtQuick 2.3
import QtQuick.Controls 1.2
import Deepin.Widgets 1.0


Row {
    id: root
    property var labelText
    property DTextInput textInput: textInput

    spacing: 20

    Text {
        text: root.labelText
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 14
        width: 120
        height: textInput.height
    }

    DTextInput{
        id: textInput
    }
}
