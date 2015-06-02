import QtQuick 2.3
import QtQuick.Controls 1.2
import Deepin.Widgets 1.0


Row {
    id: root
    property alias label: labelText.text
    property alias initValue: _spinner.initValue
    property alias min: _spinner.min
    property alias max: _spinner.max
    property alias value: _spinner.value
    property alias step: _spinner.step
    property alias spinner: _spinner
    spacing: 20

    DLabel {
        id: labelText
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        width: 120
        height: _spinner.height
    }

    DSpinner {
        id: _spinner
        width:160
    }
}
