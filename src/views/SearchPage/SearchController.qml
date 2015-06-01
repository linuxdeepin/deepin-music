import QtQuick 2.3

Item {
    property var searchPage

    Connections {
        target: searchPage
        onVisibleChanged:{
            if(visible){
                searchInput.focus = true;
                SignalManager.globalSearched(searchInput.text);
            }
        } 
    }
}