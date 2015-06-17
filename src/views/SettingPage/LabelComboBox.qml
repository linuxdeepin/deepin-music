import QtQuick 2.3
import QtQuick.Controls 1.2
import Deepin.Widgets 1.0


Row {
    id: root
    property alias label: labelText.text
    property alias items: comboBox.labels
    property alias index: comboBox.selectIndex
    property DComboBox comboBox: comboBox

    spacing: 20

    DLabel {
        id: labelText
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        width: 120
        height: comboBox.height
    }

    DComboBox {
        id: comboBox
        parentWindow: MainWindow
        menu.maxHeight: 100
        width: 160
        height: 30
        selectIndex: 0
    }
}
