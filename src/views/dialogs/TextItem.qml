import QtQuick 2.3
import DMusic 1.0
import Deepin.Widgets 1.0


Row {

    property alias keyText: keyLabel.text
    property alias valueText: valueLabel.text
    spacing: 20
    DLabel {
        id: keyLabel
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        width: 80
        color: "#888888"
    }

    DLabel {
        id: valueLabel
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        width: 150
        color: "black"
    }

}
