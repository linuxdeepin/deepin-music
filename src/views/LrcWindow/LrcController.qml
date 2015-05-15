import QtQuick 2.3

Item {
    property var toolBar
    property var lrcTextBox
    // property var canvas

    function init() {
        // LrcWorker.textChanged.connect(updateLrcText)
    }

    function updateLrcText(text, percentage, lyric_id){
        // lrcTextBox.text = text
        // print(text)
    }

    Binding {
        target: lrcTextBox
        property: 'text'
        value: {
            print(LrcWorker.currentText)
            return LrcWorker.currentText
        }
    }

    Component.onCompleted: {
        init();
    }
}