import QtQuick 2.3
import QtQuick.Controls 1.2
import Deepin.Widgets 1.0

Row {
    id: root
    property alias label: dlabel.text
    property alias shortcut: textInput.text
    property DTextInput textInput: textInput

    spacing: 20

    DLabel {
        id: dlabel
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        width: 120
        height: textInput.height
    }

    DShortcutInput{
        id: textInput
        promoteString: I18nWorker.keyBingTipMessage
    }
}
