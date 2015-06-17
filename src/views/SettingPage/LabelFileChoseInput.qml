import QtQuick 2.3
import QtQuick.Controls 1.2
import Deepin.Widgets 1.0

Row {
    id: root
    property alias label: dlabel.text
    property alias text: fileChooseInput.text
    property DFileChooseInput textInput: fileChooseInput

    spacing: 20

    signal fileChooseClicked

    DLabel {
        id: dlabel
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        width: 120
        height: textInput.height
    }

    DFileChooseInput{
        id: fileChooseInput

        onFileChooseClicked:{
            root.fileChooseClicked();
        }
    }
}
