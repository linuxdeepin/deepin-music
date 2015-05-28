import QtQuick 2.3

Item {
    property var searchPage
    property var searchInput

    Binding {
        target: searchInput
        property: 'text'
        value: SearchWorker.keyword
    }

    Connections {
        target: searchInput
        onAccepted: {
            if (searchInput.text){
                SignalManager.globalSearched(searchInput.text);
            }
        }

        onTextChanged:{
            searchTimer.restart();
        }
    }

    Timer {
        id: searchTimer
        interval: 100
        running: false
        onTriggered: {
            if (searchInput.text){
                SignalManager.globalSearched(searchInput.text);
            }
        }
    }
}