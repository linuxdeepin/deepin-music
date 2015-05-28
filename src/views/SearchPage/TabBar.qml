import QtQuick 2.3
import DMusic 1.0

ListView {
    id: root
    property var tabs:[
        {'name': I18nWorker.search_online},
        {'name': I18nWorker.search_local}
    ]
    clip: true
    orientation: ListView.Horizontal
    currentIndex: 0
    spacing: 0
    model:root.tabs
    delegate: Tab {
        width: root.width / root.count
        height: root.height
        text: ListView.view.model[index].name
        onClicked:{
            ListView.view.currentIndex = index;
        }
    }
}