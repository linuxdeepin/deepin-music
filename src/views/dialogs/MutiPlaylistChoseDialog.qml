import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"
import Deepin.Widgets 1.0

StandardDialog {
    id: mediaItem

    width: 300

    centerItem: Rectangle {
        id: playlistInputText
        width: mediaItem.width - 2
        height: 160
        
        ListView {
            id: playlistView
            property var checkNames: []
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 8
            clip: true
            highlightMoveDuration: 1
            delegate: PlaylistNameDelegate {}
            focus: true
            spacing: 2
            snapMode:ListView.SnapToItem
            DScrollBar {
                flickable: parent
                inactiveColor: 'black'
            }

            Component.onCompleted: {
                positionViewAtIndex(currentIndex, ListView.Center)
                for(var i=0; i<playlistView.count; i++){
                    checkNames[i] = false;
                }
            }
        }
    }

    Binding {
        target: playlistView
        property: 'model'
        value: PlaylistWorker.allPlaylistNames
    }

    onEntered:{
        
        SignalManager.addMutiPlaylistFlags(playlistView.checkNames)
        SignalManager.dialogClosed();
    }

    onClosed:{
        SignalManager.dialogClosed()
    }
}