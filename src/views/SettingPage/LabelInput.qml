import QtQuick 2.3
import QtQuick.Controls 1.2
import Deepin.Widgets 1.0


Row {
    id: root
    property var labelText

    spacing: 5

    Label {
        text: root.labelText
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 14
    }

    DTextInput{

    }
}
