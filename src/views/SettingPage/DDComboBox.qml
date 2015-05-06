import QtQuick 2.3
import QtQuick.Controls 1.2
import Deepin.Widgets 1.0


Row {
    id: root
    property var labelText
    property DComboBox comboBox: comboBox

    property var tmplabels: {
        var tmpArray = new Array()
        for (var i = 0; i < 100; i ++){
            tmpArray.push(i.toString())
        }

        return tmpArray
    }

    spacing: 20

    Text {
        text: root.labelText
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 14
        width: 120
        // height: comboBox.height
    }

    // DComboBox {
    //     parentWindow: MainWindow
    //     menu.maxHeight: 100
    //     labels: tmplabels
    //     width: 100
    //     height: 30
    //     text: "test"
    // }
}
